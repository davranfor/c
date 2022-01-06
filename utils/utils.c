#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <time.h>
#include "utils.h"

/* File utilities */

static long fsize(FILE *file)
{
    if (fseek(file, 0L, SEEK_END) == -1)
    {
        return -1;
    }

    long size = ftell(file);

    if ((size != -1) && (fseek(file, 0L, SEEK_SET) == -1))
    {
        return -1;
    }
    return size;
}

long file_size(const char *path)
{
    FILE *file = fopen(path, "rb");

    if (file == NULL)
    {
        return -1;
    }
    
    long size = fsize(file);

    fclose(file);
    return size;
}

static long write(const char *path, const char *str, const char *mode)
{
    FILE *file = fopen(path, mode);

    if (file == NULL)
    {
        return -1;
    }

    size_t size = strlen(str);
    long result = -1;

    if (fwrite(str, 1, size, file) == size)
    {
        result = (long)size;
    }
    fclose(file);
    return result;
}

long file_write(const char *path, const char *str)
{
    return write(path, str, "w");
}

long file_append(const char *path, const char *str)
{
    return write(path, str, "a");
}

static char *read(const char *path, const char *prefix, const char *suffix)
{
    FILE *file = fopen(path, "rb");

    if (file == NULL)
    {
        return NULL;
    }

    char *str = NULL;
    long size = fsize(file);

    if (size != -1)
    {
        size_t len = (size_t)size;
        size_t len_prefix = 0;
        size_t len_suffix = 0;

        if (prefix != NULL)
        {
            len_prefix = strlen(prefix); 
        }
        if (suffix != NULL)
        {
            len_suffix = strlen(suffix); 
        }
        if ((str = malloc(len + len_prefix + len_suffix + 1)))
        {
            if (fread(str + len_prefix, 1, len, file) == len)
            {
                if (len_prefix > 0)
                {
                    memcpy(str, prefix, len_prefix);
                }
                if (len_suffix > 0)
                {
                    memcpy(str + len + len_prefix, suffix, len_suffix);
                }
                str[len + len_prefix + len_suffix] = '\0';
            }
            else
            {
                free(str);
                str = NULL;
            }
        }
    }
    fclose(file);
    return str;
}

char *file_read(const char *path)
{
    return read(path, NULL, NULL);
}

char *file_read_with_prefix(const char *path, const char *prefix)
{
    return read(path, prefix, NULL);
}

char *file_read_with_suffix(const char *path, const char *suffix)
{
    return read(path, NULL, suffix);
}

char *file_read_quoted(const char *path, const char *prefix, const char *suffix)
{
    return read(path, prefix, suffix);
}

#define FILE_LINE_MAX 256

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

static void flush(FILE *file)
{
    int c;

    while (((c = fgetc(file)) != '\n') && (c != EOF));
}

char *file_read_buffer(FILE *file, char *str, size_t size)
{
    if (fgets(str, (int)size, file) != NULL)
    {
        char *ptr = strchr(str, '\n');

        if (ptr == NULL)
        {
            flush(file);
        }
        else
        {
            *ptr = '\0';
        }
        return str;
    }
    return NULL;
}

int file_clearerr(FILE *file)
{
    int error = ferror(file) ? 1 : 0;

    if (feof(file) || error)
    {
        clearerr(file);
    }
    return error;
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

static char *char_replace(const char *str, char chr1, char chr2)
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
            return char_replace(str, str1[0], str2[0]);
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

    if (str != NULL)
    {
        vsprintf(str, fmt, copy);
    }
    va_end(copy);
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

char *string_reverse(char *buf, const char *str)
{
    size_t len = strlen(str);

    if ((buf == NULL) && ((buf = malloc(len + 1)) == NULL))
    {
        return NULL;
    }

    // First pass. Reverse all bytes
    for (size_t head = 0, tail = len; head < tail--; head++)
    {
        char temp = str[tail];

        buf[tail] = str[head];
        buf[head] = temp;
    }
    buf[len] = '\0';

    // Second pass. Reverse multibytes
    for (char *ptr = buf; *ptr != '\0'; ptr++)
    {
        if ((*ptr & 0x80) != 0x00)
        {
            size_t mbs = 1;

            while ((ptr[mbs] & 0xc0) == 0x80)
            {
                mbs++;
            }
            for (char *end = ptr + mbs; ptr <= end;)
            {
                char temp = *end;

                *end-- = *ptr;
                *ptr++ = temp;
            }
        }
    }
    return buf;
}

char *string_convert(char *ptr, const char *str, int (*func)(int))
{
    if ((ptr == NULL) && ((ptr = malloc(strlen(str) + 1)) == NULL))
    {
        return NULL;
    }

    char *temp = ptr;

    while (*str != '\0')
    {
        *ptr++ = (char)func((unsigned char)*str++);
    }
    *ptr = '\0';
    return temp;
}

char *string_repeat(char *ptr, const char *str, size_t count)
{
    size_t len = strlen(str);

    if ((ptr == NULL) && ((ptr = malloc(count * len + 1)) == NULL))
    {
        return NULL;
    }
    if (len == 1)
    {
        for (size_t iter = 0; iter < count; iter++)
        {
            ptr[iter] = str[0];
        }
    }
    else
    {
        for (size_t iter = 0; iter < count; iter++)
        {
            memmove(ptr + (iter * len), str, len);
        }
    }
    ptr[count * len] = '\0';
    return ptr;
}

char *string_tokenize(char **str, int delimiter)
{
    char *temp = *str, *ptr = *str;
    
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
    return temp;
}

char *string_split(char **str)
{
    char *ptr = *str;

    while (isspace((unsigned char)*ptr))
    {
        ptr++;
    }
    if (*ptr == '\0')
    {
        return NULL;
    }

    char *end = ptr;

    if (*end == '"')
    {
        ptr++;
        end++;
        while ((*end != '"') && (*end != '\0'))
        {
            if ((end[0] == '\\') && (end[1] == '"'))
            {
                memmove(ptr + 1, ptr, (size_t)(end - ptr));
                ptr++;
                end++;
            }
            end++;
        }
    }
    else
    {
        while (!isspace((unsigned char)*end) && (*end != '\0'))
        {
            end++;
        }
    }
    if (*end != '\0')
    {
        *end++ = '\0';
    }
    *str = end;
    return ptr;
}

size_t string_length(const char *str)
{
    size_t len = 0;

    while (*str != 0)
    {
        if ((*str & 0xc0) != 0x80)
        {
            len++;
        }
        str++;
    }
    return len;
}

size_t string_count(const char *str, const char *substr)
{
    size_t len = strlen(substr);
    size_t count = 0;

    if (len == 1)
    {
        char chr = substr[0];

        while (*str != '\0')
        {
            if (*str == chr)
            {
                count++;
            }
            str++;
        }
    }
    else
    {
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

int string_casecmp(const char *str1, const char *str2)
{
    for (;;)
    {
        int a = tolower((unsigned char)*str1++);
        int b = tolower((unsigned char)*str2++);

        if (a != b)
        {
            return a - b;
        }
        if (a == '\0')
        {
            return 0;
        }
    }
}

/* Math utilities */

/* Random value between 0 and range - 1 */
int rrand(int range)
{
    return (int)((double)range * (rand() / (RAND_MAX + 1.0)));
}

/**
 * Returns a value divisible by multiple given a number
 * multiple must be a power of 2
 */
size_t multipleof(size_t multiple, size_t number)
{
    return (number + (multiple - 1)) & ~(multiple - 1);
}

size_t integer_length(long integer)
{
    if (integer == 0)
    {
        return 1;
    }

    size_t len = 0;

    while (integer != 0)
    {
        integer /= 10;
        len++;
    }
    return len;
}

char *integer_format(char *str, long integer, char thousands_sep)
{
    int sign = 0;

    if (integer < 0)
    {
        integer = -integer;
        *str++ = '-';
        sign = 1;
    }
    if (thousands_sep == 0)
    {
        thousands_sep = ',';
    }

    size_t len = integer_length(integer);
    char *ptr = str + len + (len - 1) / 3;
    char *end = ptr;

    while (ptr > str)
    {
        if (((end - ptr) % 4) == 3)
        {
            *--ptr = thousands_sep;
        }
        *--ptr = (char)('0' + integer % 10);
        integer /= 10;
    }
    *end = '\0';
    return str - sign;
}

char *number_format(char *str, double number, size_t decimals,
    char thousands_sep, char decimal_sep)
{
    int sign = 0;

    if (number < 0)
    {
        number = -number;
        sign = 1;
    }

    long integer, decimal = 0;

    if (decimals)
    {
        double exponent = 1.0;

        for (size_t count = 0; count < decimals; count++)
        {
            exponent *= 10;
        }
        integer = (long)number;
        decimal = (long)round((1.0 + fmod(number, 1.0)) * exponent);
        sign &= (integer || (decimal > exponent));
    }
    else
    {
        integer = (long)round(number);
        sign &= (integer > 0);
    }
    if (sign)
    {
        *str++ = '-';
    }
    str = integer_format(str, integer, thousands_sep);
    if (decimals)
    {
        char *ptr = strchr(str, '\0');

        if (decimal_sep == 0)
        {
            decimal_sep = '.';
        }
        *ptr++ = decimal_sep;
        ptr[decimals] = '\0';
        while (decimals--)
        {
            ptr[decimals] = (char)('0' + decimal % 10);
            decimal /= 10;
        }
    }
    return str - sign;
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

int leap_years(int month, int year)
{
    int years = year;

    if (month <= 2)
    {
        years--;
    }
    return (years / 4) - (years / 100) + (years / 400);
}

int year_is_leap(int year)
{
    return (((year % 4) == 0) && ((year % 100) != 0)) || ((year % 400) == 0);
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

