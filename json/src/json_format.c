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

static const char *valid_mask(const char *mask, const char *str)
{
    /**
     *  \'  quote text since next tilde (inner tildes must be escaped with \\)
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
            case '*':
                return str;
            default:
                required = 1;
                break;
        }

        int valid;

        if (function != NULL)
        {
            valid = function((unsigned char)*str);
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
            return NULL;
        }
        if (*mask == '\0')
        {
            break;
        }
        mask++;
    }
    return (*mask == *str) ? str : NULL;
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

static const char *valid_date(const char *str)
{
    const char *mask = "0000-00-00*";
    const char *valid = valid_mask(mask, str);

    if (valid)
    {
        if (is_date(strtol(&str[0], NULL, 10),
                    strtol(&str[5], NULL, 10),
                    strtol(&str[8], NULL, 10)))
        {
            return valid;
        }
    }
    return NULL;
}

int test_is_date(const char *str)
{
    const char *valid = valid_date(str);

    return valid && (*valid == '\0');
}

static int is_time_suffix(const char *str)
{
    return valid_mask("+09:00", str)
        || valid_mask("-09:00", str)
        || valid_mask("\\Z", str);
}

int test_is_time(const char *str)
{
    const char *mask = "00:00:00*";
    const char *valid = valid_mask(mask, str);

    if (valid)
    {
        if ((strtol(&str[0], NULL, 10) < 24) &&
            (strtol(&str[3], NULL, 10) < 60) &&
            (strtol(&str[6], NULL, 10) < 60))
        {
            return *valid ? is_time_suffix(valid) : 1;
        }
    }
    return 0;
}

int test_is_date_time(const char *str)
{
    const char *valid = valid_date(str);

    if (valid && (*valid == 'T'))
    {
        return test_is_time(valid + 1);
    }
    return 0;
}

static int is_hostname(const char *str, int can_end_with_dot)
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
    return is_hostname(str, 1);
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
    return is_hostname(at + 1, 0);
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
    int colons = 0, pair = 0;
    const char *mask = "xxxx:*";
    const char *valid;

    while ((valid = valid_mask(mask, str)))
    {
        str = valid;
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

    return valid_mask(mask, str) != NULL;
}

int test_is_url(const char *str)
{
    const char *mask = "\'http\'\?s://*";
    const char *valid = valid_mask(mask, str);

    if (valid && *valid)
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

