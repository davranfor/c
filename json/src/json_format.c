/*! 
 *  \brief     JSON Schema validator
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>
#include "json_format.h"

static int valid_mask(const char *mask, const char *str)
{
    /**
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
     *  *   end (returns the position if there is more text to scan or 1)
     */

    const char *ptr = str;

    while (*mask != '\0')
    {
        int (*func)(int) = NULL;
        int required = 0;

        switch (*mask)
        {
            case '\\':
                required = 1;
                mask++;
                break;
            case '\?':
                mask++;
                break;
            case '0':
                func = isdigit;
                required = 1;
                break;
            case '9':
                func = isdigit;
                break;
            case 'A':
                func = isalpha;
                required = 1;
                break;
            case 'a':
                func = isalpha;
                break;
            case 'W':
                func = isalnum;
                required = 1;
                break;
            case 'w':
                func = isalnum;
                break;
            case 'X':
                func = isxdigit;
                required = 1;
                break;
            case 'x':
                func = isxdigit;
                break;
            case '*':
                return *str ? (int)(str - ptr) : 1;
            default:
                required = 1;
                break;
        }

        int valid;

        if (func != NULL)
        {
            valid = func((unsigned char)*str);
        }
        else
        {
            valid = (*mask == *str);
        }
        if (valid)
        {
            if (*str == '\0')
            {
                break;
            }
            str++;
        }
        else if (required)
        {
            return 0;
        }
        if (*mask == '\0')
        {
            break;
        }
        mask++;
    }
    return *mask == *str;
}

static int year_is_leap(long year)
{
    return (((year % 4) == 0) && ((year % 100) != 0)) || ((year % 400) == 0);
}

static int month_days(long month, long year)
{
    static const int days[2][12] =
    {
        {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
        {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
    };
    int leap = year_is_leap(year);

    return days[leap][month - 1];
}

static int is_date(long year, long month, long day)
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

static int is_date_helper(const char *str)
{
    const char *mask = "0000-00-00*";
    int valid = valid_mask(mask, str);

    if (valid)
    {
        if (is_date(strtol(&str[0], NULL, 10),
                    strtol(&str[5], NULL, 10),
                    strtol(&str[8], NULL, 10)))
        {
            return valid;
        }
    }
    return 0;
}

int test_is_date(const char *str)
{
    return is_date_helper(str) == 1;
}

static int is_time_suffix(const char *str)
{
    return valid_mask("+09:00", str)
        || valid_mask("-09:00", str)
        || valid_mask("Z", str);
}

int test_is_time(const char *str)
{
    const char *mask = "00:00:00*";
    int valid = valid_mask(mask, str);

    if (valid)
    {
        if ((strtol(&str[0], NULL, 10) < 24) &&
            (strtol(&str[3], NULL, 10) < 60) &&
            (strtol(&str[6], NULL, 10) < 60))
        {
            return valid > 1 ? is_time_suffix(&str[valid]) : 1;
        }
    }
    return 0;
}

int test_is_date_time(const char *str)
{
    int valid = is_date_helper(str);

    if ((valid > 1) && (str[valid] == 'T'))
    {
        return test_is_time(&str[valid + 1]);    
    }
    return 0;
}

static int is_hostname_helper(const char *str, int can_end_with_dot)
{
    const char *ptr = str;
    size_t count = 0;

    while (*ptr != '\0')
    {
        if (isalnum((unsigned char)*ptr) || ((*ptr == '-') && (count > 0)))
        {
            if (++count == 64)
            {
                return 0;
            }
        }
        else if ((*ptr == '.') && (count > 0) && (ptr[-1] != '-'))
        {
            count = 0;
        }
        else
        {
            return 0;
        }
        ptr++;
    }
    if ((ptr == str) || ((ptr - str) >= 256) || (ptr[-1] == '-'))
    {
        return 0;
    }
    return (count == 0) ? can_end_with_dot : 1;
}

int test_is_hostname(const char *str)
{
    return is_hostname_helper(str, 1);
}

int test_is_email(const char *str)
{
    if ((str[0] == ' ') || (str[0] == '.'))
    {
        return 0;
    }

    const char *at = strrchr(str, '@');

    /* Maximum of 64 characters in the "local part" (before the "@") */
    if ((at == NULL) || (at == str) || (at[-1] == '.') || (at > (str + 64)))
    {
        return 0;
    }
    /* Maximum of 255 characters in the "domain part" (after the "@") */
    return is_hostname_helper(at + 1, 0);
}

int test_is_ipv4(const char *str)
{
    const char *mask = "099.099.099.099";

    if (valid_mask(mask, str))
    {
        char *ptr;

        for (int byte = 0; byte < 4; byte++)
        {
            if (strtol(str, &ptr, 10) > 255)
            {
                return 0;
            }
            str = ptr + 1;
        }
        return 1;
    }
    return 0;
}

int test_is_ipv6(const char *str)
{
    const char *mask = "xxxx:*";
    int colons = 0, pair = 0, length = 0;

    while ((length = valid_mask(mask, str)))
    {
        str += length;
        if (++colons > 7)
        {
            return 0;
        }
        /* if double colon :: */
        if (str[-1] == str[0])
        {
            /* The double colon may only be used once */
            if (pair != 0)
            {
                return 0;
            }
            pair = colons + 1;
        }
    }
    if ((colons < 2) || ((colons < 7) && (pair == 0)))
    {
        return 0;
    }
    if (str[0] == '\0')
    {
        return pair == colons;
    }
    return valid_mask("xxxx", str) || test_is_ipv4(str);
}

int test_is_uuid(const char *str)
{
    const char *mask = "XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX";

    return valid_mask(mask, str);
}

int test_is_url(const char *str)
{
    const char *mask = "http\?s://*";
    int valid = valid_mask(mask, str);

    if (valid > 1)
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

int test_regex(const char *str, const char *pattern)
{
    regex_t regex;
    int valid = 0;

    /* Compile regular expression */
    if (regcomp(&regex, pattern, REG_EXTENDED | REG_NOSUB) == 0)
    {
        /* Execute regular expression */
        valid = regexec(&regex, str, 0, NULL, 0) == 0;
    }
    /* Free regular expression */
    regfree(&regex);
    return valid;
}

