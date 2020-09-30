#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <time.h>
#include "utils.h"

/* File utilities */

#define FILE_LINE_MAX 256

static size_t file_size(FILE *file)
{
    if (fseek(file, 0L, SEEK_END) == -1)
    {
        return FILE_ERROR;
    }

    long size = ftell(file);

    if (size == -1)
    {
        return FILE_ERROR;
    }
    if (fseek(file, 0L, SEEK_SET) == -1)
    {
        return FILE_ERROR;
    }
    return (size_t)size;
}

size_t file_get_size(const char *path)
{
    FILE *file = fopen(path, "rb");

    if (file == NULL)
    {
        return FILE_ERROR;
    }
    
    size_t size = file_size(file);

    fclose(file);
    return size;
}

static char *file_data(const char *path, const char *prefix, const char *suffix)
{
    FILE *file = fopen(path, "rb");

    if (file == NULL)
    {
        return NULL;
    }

    size_t size = file_size(file);

    if (size == FILE_ERROR)
    {
        return NULL;
    }

    size_t size_prefix = 0;
    size_t size_suffix = 0;

    if (prefix != NULL)
    {
        size_prefix = strlen(prefix); 
    }
    if (suffix != NULL)
    {
        size_suffix = strlen(suffix); 
    }

    char *str = malloc(size + size_prefix + size_suffix + 1);

    if (str == NULL)
    {
        return NULL;
    }
    if (fread(str + size_prefix, 1, size, file) != size)
    {
        free(str);
        str = NULL;
    }
    else
    {
        if (size_prefix > 0)
        {
            memcpy(str, prefix, size_prefix);
        }
        if (size_suffix > 0)
        {
            memcpy(str + size, suffix, size_suffix);
        }
        str[size + size_prefix + size_suffix] = '\0';
    }
    fclose(file);
    return str;
}

char *file_read(const char *path)
{
    return file_data(path, NULL, NULL);
}

char *file_read_with_prefix(const char *path, const char *prefix)
{
    return file_data(path, prefix, NULL);
}

char *file_read_with_suffix(const char *path, const char *suffix)
{
    return file_data(path, NULL, suffix);
}

char *file_read_line(FILE *file)
{
    char str[FILE_LINE_MAX];
    char *buf = NULL;
    char *ptr = NULL;
    size_t len = 0;

    while (fgets(str, sizeof str, file) != NULL)
    {
        size_t size = sizeof str;

        ptr = strchr(str, '\n');
        if (ptr != NULL)
        {
            *ptr = '\0';
            size = (size_t)(ptr - str) + 1;
        }
        ptr = realloc(buf, len + size);
        if (ptr == NULL)
        {
            break;
        }
        memcpy(ptr + len, str, size);
        if (size != sizeof str)
        {
            return ptr;
        }
        len += size - 1;
        buf = ptr;
    }
    free(buf);
    return NULL;
}

char *file_read_buffer(FILE *file, char *str, size_t size)
{
    if (fgets(str, (int)size, file) != NULL)
    {
        char *ptr = strchr(str, '\n');

        if (ptr != NULL)
        {
            *ptr = '\0';
        }
        else
        {
            int c;

            while (((c = fgetc(file)) != '\n') && (c != EOF));
        }
        return str;
    }
    return NULL;
}

size_t file_write(const char *path, const char *str, int append)
{
    FILE *file = fopen(path, append ? "a" : "w");

    if (file == NULL)
    {
        return FILE_ERROR;
    }

    size_t size = strlen(str);

    if (fwrite(str, 1, size, file) != size)
    {
        size = FILE_ERROR;
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

static char *string_replace_char(const char *str, char chr1, char chr2)
{
    size_t len = strlen(str);
    char *ptr = malloc(len + 1);

    if (ptr == NULL)
    {
        return NULL;
    }
    while (*str != '\0')
    {
        *ptr++ = (*str == chr1) ? chr2 : *str;
        str++;
    }
    *ptr = '\0';
    return ptr - len;
}

char *string_replace(const char *str, const char *str1, const char *str2)
{
    size_t len1 = strlen(str1);
    size_t len2 = strlen(str2);
    size_t len;

    if (len1 == 0)
    {
        return NULL;
    }
    if (len1 == len2)
    {
        if (len1 == 1)
        {
            return string_replace_char(str, str1[0], str2[0]);
        }
        len = strlen(str);
    }
    else
    {
        const char *ptr = str;
        size_t count = 0;

        while (*ptr != '\0')
        {
            if ((*ptr == *str1) && (memcmp(ptr, str1, len1) == 0))
            {
                ptr += len1;
                count++;
            }
            else
            {
                ptr++;
            }
        }
        len = (size_t)(ptr - str) - (count * len1) + (count * len2);
    }

    char *ptr = malloc(len + 1);

    if (ptr == NULL)
    {
        return NULL;
    }
    while (*str != '\0')
    {
        if ((*str == *str1) && (memcmp(str, str1, len1) == 0))
        {
            memcpy(ptr, str2, len2);
            str += len1;
            ptr += len2;
        }
        else
        {
            *ptr++ = *str++;
        }
    }
    *ptr = '\0';
    return ptr - len;
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

int string_format(char *str, double value, int decimals, const char *separators)
{
    char *ptr = str;

    if (value < 0)
    {
        value = fabs(value);
        *ptr++ = '-';
    }
    if (decimals < 0)
    {
        decimals = 0;
    }

    double exponent = pow(10, decimals);
    double integral, fractional;

    value = round(value * exponent) / exponent;
    fractional = modf(value, &integral);
    fractional = round(fractional * exponent);

    int digits = 1;

    if (integral > 0)
    {
        digits += (int)log10(integral);
        digits += digits / 3;
    }
    ptr += digits;
    for (int count = 1; count <= digits; count++)
    {
        if ((count % 4) == 0)
        {
            ptr[-count] = separators[0];
            count++;
        }
        ptr[-count] = (char)('0' + fmod(integral, 10));
        integral /= 10;
    }
    if (decimals > 0)
    {
        *ptr++ = separators[1];
        for (int count = decimals - 1; count >= 0; count--)
        {
            ptr[count] = (char)('0' + fmod(fractional, 10));
            fractional /= 10;
        }
        ptr += decimals;
    }
    *ptr = '\0';
    return (int)(ptr - str);
}

char *string_tokenize(char **str, int delimiter)
{
    char *result = *str, *ptr = *str;
    
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
        *str = ptr + 1;
        *ptr = '\0';
    }
    else
    {
        *str = NULL;
    }
    return result;
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

static size_t string_count_char(const char *str, char chr)
{
    size_t count = 0;

    while (*str != '\0')
    {
        if (*str++ == chr)
        {
            count++;
        }
    }
    return count;
}

size_t string_count(const char *str, const char *substr)
{
    size_t len = strlen(substr);

    if (len == 0)
    {
        return 0;
    }
    if (len == 1)
    {
        return string_count_char(str, substr[0]);
    }

    size_t count = 0;

    while (*str != '\0')
    {
        if ((*str == *substr) && (memcmp(str, substr, len) == 0))
        {
            str += len;
            count++;
        }
        else
        {
            str++;
        }
    }
    return count;
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

int date_is_valid(int day, int month, int year)
{
    if ((year < 0) || (year > 9999))
    {
        return 0;
    }
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

/* Random value between 0 and range - 1 */
int rrand(int range)
{
    return (int)((double)range * (rand() / (RAND_MAX + 1.0)));
}

