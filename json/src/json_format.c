/*!
 *  \brief     json and json-schema library for C
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include "json_macros.h"
#include "json_format.h"

static const char *test_mask(const char *text, const char *mask)
{
    /**
     *  \'  quote text until next quote (inner quotes must be escaped with \\)
     *  \\  next character is a literal (not a function) (required)
     *  \?  next character is a literal (not a function) (optional)
     *  0   isdigit (required)
     *  9   isdigit (optional)
     *  A   isalpha (required)
     *  a   isalpha (optional)
     *  W   isalnum (required)
     *  w   isalnum (optional)
     *  X   isxdigit (required)
     *  x   isxdigit (optional)
     *  *   return the string at this position
     */

    int quoted = 0;

    while (*mask != '\0')
    {
        int (*function)(int) = NULL;
        int required = 0;

        if (quoted)
        {
            if (*mask == '\'')
            {
                quoted = 0;
                mask++;
                continue;
            }
            if (*mask == '\\')
            {
                mask++;
            }
            required = 1;
        }
        else switch (*mask)
        {
            case '\'':
                quoted = 1;
                mask++;
                continue;
            case '\\':
                required = 1;
                mask++;
                break;
            case '\?':
                mask++;
                break;
            case '0':
                function = isdigit;
                required = 1;
                break;
            case '9':
                function = isdigit;
                break;
            case 'A':
                function = isalpha;
                required = 1;
                break;
            case 'a':
                function = isalpha;
                break;
            case 'W':
                function = isalnum;
                required = 1;
                break;
            case 'w':
                function = isalnum;
                break;
            case 'X':
                function = isxdigit;
                required = 1;
                break;
            case 'x':
                function = isxdigit;
                break;
            default:
                required = 1;
                break;
            case '*':
                return text;
        }

        int valid = function
            ? function((unsigned char)*text)
            : *text == *mask;

        if (valid)
        {
            if (*text == '\0')
            {
                break;
            }
            text++;
        }
        else if (required)
        {
            return NULL;
        }
        if (*mask != '\0')
        {
            mask++;
        }
    }
    return (*text == *mask) ? text : NULL;
}

static int year_is_leap(int year)
{
    return (((year % 4) == 0) && ((year % 100) != 0)) || ((year % 400) == 0);
}

static int month_days(int month, int year)
{
    static const int days[2][12] =
    {
        {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
        {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
    };
    int leap = year_is_leap(year);

    return days[leap][month - 1];
}

static int is_date(int year, int month, int day)
{
    if ((month < 1) || (month > 12))
    {
        return 0;
    }
    if ((day < 1) || (day > month_days(month, year)))
    {
        return 0;
    }
    return 1;
}

static const char *test_date(const char *str)
{
    const char *date = test_mask(str,"0000-00-00*");

    if (date != NULL)
    {
        if (is_date((int)strtol(&str[0], NULL, 10),
                    (int)strtol(&str[5], NULL, 10),
                    (int)strtol(&str[8], NULL, 10)))
        {
            return date;
        }
    }
    return NULL;
}

int test_is_date(const char *str)
{
    return (str = test_date(str)) && (*str == '\0');
}

static int is_time_suffix(const char *str)
{
    return test_mask(str, "+09:00")
        || test_mask(str, "-09:00")
        || test_mask(str, "\\Z");
}

int test_is_time(const char *str)
{
    const char *time = test_mask(str, "00:00:00*");

    if (time != NULL)
    {
        if ((strtol(&str[0], NULL, 10) < 24) &&
            (strtol(&str[3], NULL, 10) < 60) &&
            (strtol(&str[6], NULL, 10) < 60))
        {
            return *time ? is_time_suffix(time) : 1;
        }
    }
    return 0;
}

int test_is_date_time(const char *str)
{
    if ((str = test_date(str)) && (*str == 'T'))
    {
        return test_is_time(str + 1);
    }
    return 0;
}

static const char *test_hostname(const char *str)
{
    // Must start with a digit or alpha character
    if (!is_alnum(*str))
    {
        return NULL;
    }

    int label_length = 0, length = 0;

    while (*str != '\0')
    {
        // Don't allow "--" or "-." or ".-" or ".."
        if (((str[0] == '-') || (str[0] == '.')) &&
            ((str[1] == '-') || (str[1] == '.')))
        {
            return NULL;
        }
        // Each label must be between 1 and 63 characters long
        // The entire hostname (including the delimiting dots
        // but not a trailing dot) has a maximum of 253 chars
        if ((*str == '-') || is_alnum(*str))
        {
            if ((label_length++ == 63) || (length >= 253))
            {
                return NULL;
            }
        }
        else if (*str == '.')
        {
            label_length = 0;
        }
        else
        {
            return NULL;
        }
        length++;
        str++;
    }
    // Can not end with hyphen
    return (str[-1] != '-') ? str : NULL;
}

int test_is_hostname(const char *str)
{
    return test_hostname(str) ? 1 : 0;
}

int test_is_email(const char *str)
{
    // The local part can not start with space, dot or at symbol
    if ((*str == ' ') || (*str == '.') || (*str == '@'))
    {
        return 0;
    }

    int mbs = 0;

    // Max. 63 UTF8 chars in the local part
    while ((*str != '@') && (*str != '\0'))
    {
        if (is_utf8(*str) && (mbs++ == 63))
        {
            return 0;
        }
        str++;
    }
    // The local part can not end with a dot
    if ((str[0] != '@') || (str[-1] == '.'))
    {
        return 0;
    }
    // The domain part can not end with a dot
    return (str = test_hostname(str + 1)) && (str[-1] != '.');
}

int test_is_ipv4(const char *str)
{
    if (!test_mask(str, "099.099.099.099"))
    {
        return 0;
    }
    for (int byte = 0; byte < 4; byte++)
    {
        char *end;

        if (strtol(str, &end, 10) < 256)
        {
            str = end + 1;
        }
        else
        {
            return 0;
        }
    }
    return 1;
}

/**
 * ipv6 addresses can consist of:
 * - 2-6 ipv6 segments abbreviated with a double colon with or without ipv4
 * - 6 ipv6 segments separated by single colons and required ipv4
 * - 6-8 ipv6 segments abbreviated with a double colon without ipv4
 * - 8 ipv6 segments separated by single colons without ipv4
 */
int test_is_ipv6(const char *str)
{
    const char *mask = "xxxx:*", *end = str, *next;
    int colons = 0, abbrv = 0;

    while ((next = test_mask(end, mask)) && (colons < 7))
    {
        // Double colon may only be used once
        if ((colons++ > 0) && (next == end + 1))
        {
            if (abbrv++ > 0)
            {
                return 0;
            }
        }
        end = next;
    }
    // Can not start with a single colon (except abbr.v '::')
    if ((str[0] == ':') && (str[1] != ':'))
    {
        return 0;
    }
    // 6 ipv6 segments separated by single colons and required ipv4
    if ((colons == 5) && (abbrv == 0))
    {
        return test_is_ipv4(end);
    }
    // 6-8 ipv6 segments abbreviated with double colon without ipv4
    if ((colons >= 5) && (abbrv == 1))
    {
        return test_mask(end, "xxxx") ? 1 : 0;
    }
    // 8 ipv6 segments separated by single colons without ipv4
    if ((colons == 7) && (abbrv == 0))
    {
        return test_mask(end, "Xxxx") ? 1 : 0;
    }
    // 2-6 segments abbreviated with a double colon with or without ipv4
    return (abbrv == 1) && (test_mask(end, "xxxx") || test_is_ipv4(end));
}

int test_is_uuid(const char *str)
{
    return test_mask(str, "XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX") ? 1 : 0;
}

int test_is_url(const char *str)
{
    const char *url = test_mask(str, "\'http\'\?s://*");

    if (url && *url)
    {
        const char *allow = "abcdefghijklmnopqrstuvwxyz"
                            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                            "0123456789"
                            "-._~:/?#[]@!$&'()*+,;%=";

        size_t end = strspn(str, allow);

        // Maximum of 2048 characters
        return (end <= 2048) && (str[end] == '\0');
    }
    return 0;
}

int test_regex(const char *text, const char *pattern)
{
    regex_t regex;
    int valid = 0;

    if (regcomp(&regex, pattern, REG_EXTENDED | REG_NOSUB) == 0)
    {
        valid = regexec(&regex, text, 0, NULL, 0) == 0;
    }
    regfree(&regex);
    return valid;
}

