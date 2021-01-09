#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <time.h>
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
    
    str = string_replace("El camion de Ramon", "o", "ó");
    if (str != NULL)
    {
        printf("%s ", str);
        printf("<- 'ó' appears %zu times\n", string_count(str, "ó"));
        free(str);
    }

    str = string_reverse("El camión de Ramón vale 1000 €");
    if (str != NULL)
    {
        printf("<%s>\n", str);
        free(str);
    }

    str = string_repeat("€", 5);
    if (str != NULL)
    {
        printf("<%s>\n", str);
        free(str);
    }

    double number = 1234567.890;
    char money[64];

    string_format(money, number, 2, ",.");
    printf("%.3f as money = %s\n", number, money);
    string_format(money, number, 0, ",");
    printf("%.3f as real = %s\n", number, money);

    char arr[] = "one,two,three";
    char *ptr = arr;

    while ((str = string_tokenize(&ptr, ',')))
    {
        printf("%s\n", str);
    }
}

static int eof_handler(FILE *file)
{
    int error = file_error(file) ? 1 : 0;

    if (file_eof(file) || error)
    {
        clearerr(file);
        return error;
    }
    return 2; // Unhandled error
}

static void sample_files(void)
{
    puts("Sample files:");

    const char *path = "test.txt";
    size_t size;

    size = file_write(path, "Enter text (Press CTRL + D to stop)\n", FILE_TRUNCATE);
    if (size == FILE_ERROR)
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
        printf("<%s>\n", str);
        free(str);
    }
    if (eof_handler(stdin) != 0)
    {
        perror("file_read_line");
    }

    char buf[10];

    puts("Enter text (Press CTRL + D to stop)");
    while (file_read_buffer(stdin, buf, sizeof buf))
    {
        printf("<%s>\n", buf);
    }
    if (eof_handler(stdin) != 0)
    {
        perror("file_read_buffer");
    }
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
    printf("Days between 01/02/2000 and 01/02/2004: %d\n",
        days_diff(1, 2, 2000, 1, 2, 2004));
}

/* Knuth shuffle algorithm */
static void randomize(int arr[], int size)
{
    while (size > 1)
    {
        int item = rrand(size--);
        int temp = arr[size];

        arr[size] = arr[item];
        arr[item] = temp;
    }
}

#define print_array(format, array, elems)   \
do                                          \
{                                           \
    for (int ___ = 0; ___ < elems; ___++)   \
    {                                       \
        printf(format, array[___]);         \
    }                                       \
    printf("\n");                           \
} while (0)

static void sample_misc(void)
{
    // Randomize
    {
        enum {N = 5};
        int arr[N];

        for (int iter = 0; iter < N; iter++)
        {
            arr[iter] = iter;
        }
        randomize(arr, N);
        printf("Random numbers between 0 and %d:\n", N);
        print_array("%d ", arr, N);
    }
    // Multidimensional array in linear storage (Row-major order)
    {
        enum {rows = 5, cols = 3};
        int arr[rows * cols];
        int row, col, index;

        puts("Row-major order:");
        for (row = 0; row < rows; row++)
        {
            for (col = 0; col < cols; col++)
            {
                index = cols * row + col;
                arr[index] = index;
                printf("[%d][%d] = %d\n", row, col, index);
            }
        }
        row = rows / 2, col = cols / 2;
        printf("Value of row %d col %d is %d\n", row, col, arr[cols * row + col]);
        index = rows * cols / 2;
        printf("Position of index %d is row %d col %d\n", index, index / cols, index % cols);
    }
}

int main(void)
{
    srand((unsigned)time(NULL));
    setlocale(LC_CTYPE, "");
    sample_strings();
    sample_files();
    sample_dates();
    sample_misc();
    return 0;
}

