#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "utils.h"
#include "wutils.h"
#include "signed_maths.h"

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

    str = string_repeat(NULL, "€", 5);
    if (str != NULL)
    {
        printf("<%s>\n", str);
        free(str);
    }

    str = string_trim(" \t En un lugar de la Mancha ... \n ");
    if (str != NULL)
    {
        printf("<%s>\n", str);
        printf("<%s>\n", string_convert(str, str, toupper));
        free(str);
    }

    str = string_convert(NULL, "En un lugar de la Mancha ...", tolower);
    if (str != NULL)
    {
        printf("<%s>\n", str);
        free(str);
    }

    str = string_wconvert(NULL, "El camión de María vale 1000 €", towupper);
    if (str != NULL)
    {
        printf("<%s>\n", str);
        printf("<%s>\n", string_wconvert(str, str, towlower));
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

    str = string_reverse(NULL, "El camión de Ramón vale 1000 €");
    if (str != NULL)
    {
        printf("<%s>\n", str);
        free(str);
    }

    if (string_casecmp("aBcDe", "AbCdE") != 0)
    {
        puts("Strings are different");
    }

    if (string_wcasecmp("El camión de María", "eL camión de marÍa") != 0)
    {
        puts("Strings are different");
    }

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

#define print_array(format, array, elems)       \
do                                              \
{                                               \
    for (size_t ___ = 0; ___ < elems; ___++)    \
    {                                           \
        printf(format, array[___]);             \
    }                                           \
    printf("\n");                               \
} while (0)

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

static size_t deletion(int arr[], size_t size, size_t item)
{
    if (item < size)
    {
        memmove(&arr[item], &arr[item + 1], (--size - item) * sizeof *arr);
    }
    return size;
}

static void sample_maths(void)
{
    char str[64];

    number_format(str, 1234567.890, 2, '.', ',');
    printf("Number format: %s\n", str);
    integer_format(str, 123456789, '.');
    printf("Integer format: %s\n", str);

    // Randomize and deletion
    {
        /* Randomize */
        enum {N = 5};
        int arr[N];

        for (int iter = 0; iter < N; iter++)
        {
            arr[iter] = iter;
        }
        randomize(arr, N);
        printf("Random numbers between 0 and %d:\n", N);
        print_array("%d ", arr, N);

        /* Deletion */
        size_t item = (size_t)rrand(N);
        size_t size = deletion(arr, N, item);

        printf("Deleting %zu:\n", item);
        print_array("%d ", arr, size);
    }
    // Multidimensional array in linear storage (Row-major order)
    {
        enum {rows = 5, cols = 3};
        int arr[rows * cols];
        int row, col, item;

        puts("Row-major order:");
        for (row = 0; row < rows; row++)
        {
            for (col = 0; col < cols; col++)
            {
                item = cols * row + col;
                arr[item] = item;
                printf("[%d][%d] = %d\n", row, col, item);
            }
        }
        row = rows / 2, col = cols / 2;
        printf("Value of row %d col %d is %d\n", row, col, arr[cols * row + col]);
        item = rows * cols / 2;
        printf("Position of item %d is row %d col %d\n", item, item / cols, item % cols);
    }
    // Multiples of power of 2 numbers
    for (int iter = 0; iter < 10; iter++)
    {
        size_t multiple = 1U << (rand() % 16); // Power of 2
        size_t number = (size_t)(rand() / 2048);

        printf(
            "number = %zu | multiple = %zu | result = %zu\n",
            number, multiple, multipleof(multiple, number)
        );
    }
    // Safe signed overflow-underflow
    printf("Wrap-around protected %d + 42 = %d\n", INT_MAX, int_wrap_add(INT_MAX, 42));
    printf("      Range protected %d + 42 = %d\n", INT_MAX, int_range_add(INT_MAX, 42));
    // printf("   Abort protected %d + 42 = %d\n", INT_MAX, int_safe_add(INT_MAX, 42));
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

int main(void)
{
    srand((unsigned)time(NULL));
    setlocale(LC_CTYPE, "");
    sample_strings();
    sample_files();
    sample_maths();
    sample_dates();
    return 0;
}

