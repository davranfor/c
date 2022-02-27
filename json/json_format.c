/*! 
 *  \brief     JSON Schema validator
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#include <stdio.h>
#include <stdlib.h>
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

int test_is_date(const char *str)
{
    int valid = isdigit((unsigned char)str[0]) &&
                isdigit((unsigned char)str[1]) &&
                isdigit((unsigned char)str[2]) &&
                isdigit((unsigned char)str[3]) &&
                (str[4] == '-') &&
                isdigit((unsigned char)str[5]) &&
                isdigit((unsigned char)str[6]) &&
                (str[7] == '-') &&
                isdigit((unsigned char)str[8]) &&
                isdigit((unsigned char)str[9]);

    if (valid)
    {
        return is_date(
            strtol(&str[0], NULL, 10),
            strtol(&str[5], NULL, 10),
            strtol(&str[8], NULL, 10)
        );
    }
    return 0;
}

int test_is_email(const char *str)
{
    if (!isalpha((unsigned char)*str++))
    {
        return 0;
    }

    int at = 0, dot = 0, pos = 0;

    while (*str != '\0')
    {
        pos++;
        if (*str == '@')
        {
            if ((at != 0) || (dot != 0))
            {
                return 0;
            }
            at = pos;
        }
        else if (*str == '.')
        {
            if ((at == 0) || (dot != 0) || (at == pos -1))
            {
                return 0;
            }
            dot = pos;
        }
        else if (!isalnum((unsigned char)*str))
        {
            return 0;
        }
        str++;
    }
    if ((at == 0) || (dot == 0) || (pos == dot))
    {
        return 0;
    }
    return 1;
}

