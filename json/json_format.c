/*! 
 *  \brief     JSON Schema validator
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "json_format.h"

static int valid_mask(const char *mask, const char *str)
{
    /*
     *  +   repeat while next intruction match
     *  \\  next character is a literal (not a function) (required)
     *  \?  next character is a literal (not a function) (optional)
     *  0   isdigit (required)
     *  9   isdigit (optional)
     *  A   isalpha (required)
     *  a   isalpha (optional)
     *  T   isalnum (required)
     *  t   isalnum (optional)
     *  X   isxdigit (required)
     *  x   isxdigit (optional)
     *  *   end (returns the position if there is more text to scan or 1)
     */

    const char *ptr = str;
    int required = 0;
    int repeat = 0;

    while (*mask != '\0')
    {
        int (*func)(int) = NULL;

        switch (*mask)
        {
            case '+':
                repeat = 1;
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
            case 'T':
                func = isalnum;
                required = 1;
                break;
            case 't':
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
                return *str ? 1 : (int)(str - ptr);
            default:
                break;
        }

        int match;

        if (func != NULL)
        {
            match = func((unsigned char)*str);
        }
        else
        {
            match = (*mask == *str);
        }
        if (match)
        {
            if (*str != '\0')
            {
                str++;
            }
            else
            {
                break;
            }
            if (repeat)
            {
                if (func)
                {
                    while (func((unsigned char)*str))
                    {
                        str++;
                    }
                }
                else
                {
                    while (*mask == *str)
                    {
                        str++;
                    }
                }
            }
        }
        else if (required)
        {
            return 0;
        }
        if (*mask != '\0')
        {
            mask++;
        }
        else
        {
            break;
        }
        required = 0;
        repeat = 0;
    }
    return (*str == *mask);
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
    const char *mask = "\\+00:00";

    if (valid_mask(mask, str))
    {
        return (strtol(&str[1], NULL, 10) < 60)
            && (strtol(&str[4], NULL, 10) < 60);
    }
    return 0;
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

int test_is_email(const char *str)
{
    const char *mask = "A+t@+T.+T";

    return valid_mask(mask, str);
}

int test_is_ipv4(const char *str)
{
    const char *mask = "099.099.099.099";

    if (valid_mask(mask, str))
    {
        char *ptr;

        for (int i = 0; i < 4; i++)
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
    int colons = 0, pair = 0, len = 0;

    while ((len = valid_mask(mask, str)))
    {
        str += len;
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

