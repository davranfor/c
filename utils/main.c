#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include "utils.h"

static void sample_strings(void)
{
    char *str;

    puts("Sample strings:");
    str = string_clone("Hello World!");
    if (str != NULL)
    {
        printf("<%s>\n", str);
        free(str);
    }

    str = string_print("The value of pi is %g", 3.14);
    if (str != NULL)
    {
        printf("<%s>\n", str);
        free(str);
    }

    str = string_trim(" \t En un lugar de la mancha ... \n ");
    if (str != NULL)
    {
        printf("<%s>\n", str);
        free(str);
    }

    str = "María";
    printf("\"%s\" -> length = %zu\n", str, string_length(str));
}

static void sample_files(void)
{
    puts("Sample files:");

    const char *path = "test.txt";
    size_t size;

    size = file_write(path, "Enter text (Press CTRL + D to stop)\n>", FILE_TRUNCATE);
    if (size == FILE_WRITE_ERROR)
    {
        perror("file_write");
        fprintf(stderr, "%s\n", path);
    }

    char *str;

    str = file_read(path);
    if (str == NULL)
    {
        perror("file_read");
        fprintf(stderr, "%s\n", path);
    }
    else
    {
        printf("%s", str);
        free(str);
    }

    while ((str = file_read_line(stdin)))
    {
        printf("<%s>\n>", str);
        free(str);
    }
    if (file_error(stdin) != 0)
    {
        perror("file_read_line");
        fprintf(stderr, "File error #%d\n", file_error(stdin));
    }
    puts("");
}

static void sample_dates(void)
{
    puts("Sample dates:");

    int day, month, year;
    int hour, minutes, seconds;

    today(&day, &month, &year);
    now(&hour, &minutes, &seconds);
    printf("Date (dd/mm/yyyy hh:mm:ss): %02d/%02d/%d %02d:%02d:%02d\n",
        day, month, year, hour, minutes, seconds);
    printf("Day of week: %d\n", day_of_week(day, month, year));
    printf("Day of week (ISO 8601): %d\n", ISO_day_of_week(day, month, year));
    printf("Day of year: %d\n", day_of_year(day, month, year));
    printf("Week of month: %d\n", week_of_month(day, month, year));
    printf("Week of year: %d\n", week_of_year(day, month, year));
    printf("Days in this month: %d\n", month_days(month, year));
    day_add(&day, &month, &year, -1);
    printf("Yesterday: %02d/%02d/%d\n", day, month, year);
    day_add(&day, &month, &year, +2);
    printf("Tomorrow: %02d/%02d/%d\n", day, month, year);
}

int main(void)
{
    setlocale(LC_CTYPE, "");
    sample_strings();
    sample_files();
    sample_dates();
    return 0;
}

