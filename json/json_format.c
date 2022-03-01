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

static int test_mask(const char *mask, const char *str)
{
    /*
     * 0 isdigit (required)
     * 9 isdigit (optional)
     * A isalpha (required)
     * a isalpha (optional)
     * T isalnum (required)
     * t isalnum (optional)
     * + repeat while function matches
     * * end (valid)
     */

    int repeat = 0;

    while (*mask)
    {
        int (*func)(int) = NULL;
        int required = 1;

        switch (*mask)
        {
            case '0':
                func = isdigit;
                break;
            case '9':
                func = isdigit;
                required = 0;
                break;
            case 'A':
                func = isalpha;
                break;
            case 'a':
                func = isalpha;
                required = 0;
                break;
            case 'T':
                func = isalnum;
                break;
            case 't':
                func = isalnum;
                required = 0;
                break;
            case '+':
                mask++;
                repeat = 1;
                continue;
            case '*':
                return 1;
            default:
                if (*mask != *str)
                {
                    return 0;
                }
                break;
        }

        int match = func ? func((unsigned char)*str) : 1;

        if (match)
        {
            str++;
            if (repeat && func)
            {
                while (func((unsigned char)*str))
                {
                    str++;
                }
            }
        }
        else if (required)
        {
            return 0;
        }
        repeat = 0;
        mask++;
    }
    return *mask == *str;
}

int test_is_date(const char *str)
{
    const char *mask = "0000-00-00";

    if (test_mask(mask, str))
    {
        return is_date(
            strtol(&str[0], NULL, 10),
            strtol(&str[5], NULL, 10),
            strtol(&str[8], NULL, 10)
        );
    }
    return 0;
}

int test_is_time(const char *str)
{
    const char *mask = "00:00:00";

    if (test_mask(mask, str))
    {
        return (strtol(&str[0], NULL, 10) < 24)
            && (strtol(&str[3], NULL, 10) < 60)
            && (strtol(&str[6], NULL, 10) < 60);
    }
    return 0;
}

int test_is_email(const char *str)
{
    const char *mask = "A+t@+T.+T";

    return test_mask(mask, str);
}

int test_is_ip_address(const char *str)
{
    const char *mask = "099.099.099.099";

    if (test_mask(mask, str))
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

