#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include "utils.h"

/* File utilities */

#define FILE_LINE_MAX 256

static long f_get_size(FILE *file)
{
    if (fseek(file, 0L, SEEK_END) == -1)
    {
        return -1;
    }

    long size = ftell(file);

    if (size == -1)
    {
        return -1;
    }
    if (fseek(file, 0L, SEEK_SET) == -1)
    {
        return -1;
    }
    return size;
}

long file_get_size(const char *path)
{
    FILE *file = fopen(path, "r");

    if (file == NULL)
    {
        return -1;
    }
    
    long size = f_get_size(file);

    fclose(file);
    return size;
}

static char *f_get_mem(FILE *file, size_t size, size_t prefix, size_t suffix)
{
    char *str = malloc(size + prefix + suffix + 1);

    if (str == NULL)
    {
        return NULL;
    }
    if (fread(str + prefix, 1, size, file) != size)
    {
        free(str);
        return NULL;
    }
    str[size + prefix + suffix] = '\0';
    return str;
}

static char *f_read(FILE *file)
{
    long size = f_get_size(file);

    if (size == -1)
    {
        return NULL;
    }
    return f_get_mem(file, (size_t)size, 0, 0);
}

static char *f_read_with_prefix(FILE *file, const char *prefix)
{
    long size = f_get_size(file);

    if (size == -1)
    {
        return NULL;
    }

    size_t len = strlen(prefix);
    char *str = f_get_mem(file, (size_t)size, len, 0);

    if (str != NULL)
    {
        memcpy(str, prefix, len);
    }
    return str;
}

static char *f_read_with_suffix(FILE *file, const char *suffix)
{
    long size = f_get_size(file);

    if (size == -1)
    {
        return NULL;
    }

    size_t len = strlen(suffix);
    char *str = f_get_mem(file, (size_t)size, 0, len);

    if (str != NULL)
    {
        memcpy(str + size, suffix, len);
    }
    return str;
}

char *file_read(const char *path)
{
    FILE *file = fopen(path, "r");

    if (file == NULL)
    {
        return NULL;
    }

    char *str = f_read(file);

    fclose(file);
    return str;
}

char *file_read_with_prefix(const char *path, const char *prefix)
{
    FILE *file = fopen(path, "r");

    if (file == NULL)
    {
        return NULL;
    }

    char *str = f_read_with_prefix(file, prefix);

    fclose(file);
    return str;
}

char *file_read_with_suffix(const char *path, const char *suffix)
{
    FILE *file = fopen(path, "r");

    if (file == NULL)
    {
        return NULL;
    }

    char *str = f_read_with_suffix(file, suffix);

    fclose(file);
    return str;
}

char *file_read_line(FILE *file)
{
    char str[FILE_LINE_MAX];
    char *buf = NULL;
    char *ptr = NULL;
    size_t size = 0;

    while (fgets(str, sizeof str, file) != NULL)
    {
        size_t len = sizeof str;

        ptr = strchr(str, '\n');
        if (ptr != NULL)
        {
            len = (size_t)(ptr - str) + 1;
            *ptr = '\0';
        }
        ptr = realloc(buf, size + len);
        if (ptr == NULL)
        {
            break;
        }
        memcpy(ptr + size, str, len);
        if (len != sizeof str)
        {
            return ptr;
        }
        size += len - 1;
        buf = ptr;
    }
    free(buf);
    return NULL;
}

size_t file_write(const char *path, const char *str, int append)
{
    FILE *file = fopen(path, append ? "a" : "w");

    if (file == NULL)
    {
        return FILE_WRITE_ERROR;
    }

    size_t size = strlen(str);

    if (fwrite(str, 1, size, file) != size)
    {
        size = FILE_WRITE_ERROR;
    }
    fclose(file);
    return size;
}

int file_error(FILE *file)
{
    if (feof(file))
    {
        return 0;
    }
    return ferror(file);
}

/* String utilities */

char *string_clone(const char *str)
{
    size_t size = strlen(str) + 1;
    char *ptr = malloc(size);

    if (ptr == NULL)
    {
        return NULL;
    }
    return memcpy(ptr, str, size);
}

char *string_slice(const char *str, size_t start, size_t end)
{
    size_t diff = end - start;
    char *ptr = malloc(diff + 1);

    if (ptr == NULL)
    {
        return NULL;
    }
    memcpy(ptr, str + start, diff);
    ptr[diff] = '\0';
    return ptr;
}

static char *string_vprint(const char *fmt, va_list args)
{
    va_list copy;

    va_copy(copy, args);

    size_t len = (size_t)vsnprintf(NULL, 0, fmt, args);
    char *str = malloc(len + 1);

    if (str == NULL)
    {
        return NULL;
    }
    vsprintf(str, fmt, copy);
    return str;
}

char *string_print(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);

    char *str = string_vprint(fmt, args);

    va_end(args);
    return str;
}

char *string_trim(const char *str)
{
    str += string_lskip(str, isspace);

    return string_slice(str, 0, string_rskip(str, isspace));
}

char *string_ltrim(const char *str)
{
    return string_clone(str + string_lskip(str, isspace));
}

char *string_rtrim(const char *str)
{
    return string_slice(str, 0, string_rskip(str, isspace));
}

char *string_tokenize(char **str, int delimiter)
{
    char *res = *str, *ptr = res;
    
    if (ptr == NULL)
    {
        return NULL;
    }
    while ((*ptr != delimiter) && (*ptr != '\0'))
    {
        ptr++;
    }
    if (*ptr != '\0')
    {
        *ptr = '\0';
        *str = ptr + 1;
    }
    else
    {
        *str = NULL;
    }
    return res;
}

size_t string_length(const char *str)
{
    size_t len = 0;

    while (*str != 0)
    {
        if ((*str++ & 0xc0) != 0x80)
        {
            len++;
        }
    }
    return len;
}

size_t string_lskip(const char *str, int func(int))
{
    size_t pos = 0;

    while (func((unsigned char)str[pos]))
    {
        pos++;
    }
    return pos;
}

size_t string_rskip(const char *str, int func(int))
{
    size_t pos = strlen(str);

    while ((pos > 0) && func((unsigned char)str[pos - 1]))
    {
        pos--;
    }
    return pos;
}

/* Date utilities */

void today(int *day, int *month, int *year)
{
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    *day = tm.tm_mday;
    *month = tm.tm_mon + 1;
    *year = tm.tm_year + 1900;
}

void now(int *hour, int *minutes, int *seconds)
{
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    *hour = tm.tm_hour;
    *minutes = tm.tm_min;
    *seconds = tm.tm_sec;
}

void day_add(int *day, int *month, int *year, int days)
{
    struct tm tm = {0};

    tm.tm_mday = *day + days;
    tm.tm_mon = *month - 1;
    tm.tm_year = *year - 1900;

    mktime(&tm);

    *day = tm.tm_mday;
    *month = tm.tm_mon + 1;
    *year = tm.tm_year + 1900;
}

int days_diff(int day1, int month1, int year1,
              int day2, int month2, int year2)
{
    static const int days[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
    long int days1 = (year1 * 365) + day1;
    long int days2 = (year2 * 365) + day2;

    days1 += days[month1 - 1] + leap_years(month1, year1);
    days2 += days[month2 - 1] + leap_years(month2, year2);
    return (int)(days2 - days1);
}

/**
 * Tomohiko Sakamoto's Algorithm
 * Sunday = 0 ... Saturday = 6
 */
int day_of_week(int day, int month, int year)
{
    static const int offset[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};

    year -= month < 3;
    return (year + year / 4 - year / 100 + year / 400 + offset[month - 1] + day) % 7;
}

/**
 * ISO 8601 date and time standard
 * Monday = 1 ... Sunday = 7
 */
int ISO_day_of_week(int day, int month, int year)
{
    static const int offset[] = {6, 2, 1, 4, 6, 2, 4, 0, 3, 5, 1, 3};

    year -= month < 3;
    return (year + year / 4 - year / 100 + year / 400 + offset[month - 1] + day) % 7 + 1;
}

int day_of_year(int day, int month, int year)
{
    static const int days[2][12] =
    {
        {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
        {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335}
    };
    int leap = year_is_leap(year);

    return days[leap][month - 1] + day;
}

int week_of_month(int day, int month, int year)
{
    return (day - ISO_day_of_week(day, month, year) + 10) / 7;
}

int week_of_year(int day, int month, int year)
{
    return (day_of_year(day, month, year) - ISO_day_of_week(day, month, year) + 10) / 7;
}

int month_days(int month, int year)
{
    static const int days[2][12] =
    {
        {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
        {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
    };
    int leap = year_is_leap(year);

    return days[leap][month - 1];
}

int year_is_leap(int year)
{
    return (((year % 4) == 0) && ((year % 100) != 0)) || ((year % 400) == 0);
}

int leap_years(int month, int year)
{
    int years = year;

    if (month <= 2)
    {
        years--;
    }
    return (years / 4) - (years / 100) + (years / 400);
}

