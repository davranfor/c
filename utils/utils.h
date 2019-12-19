#ifndef UTILS_H
#define UTILS_H

#include <ctype.h>

/* ************************************************************************* */
/* File utilities                                                            */
/* ************************************************************************* */

#define FILE_TRUNCATE 0
#define FILE_APPEND 1

#define FILE_WRITE_ERROR ((size_t)-1)

long file_get_size(const char *);
char *file_read(const char *);
char *file_read_with_prefix(const char *, const char *);
char *file_read_line(FILE *);
size_t file_write(const char *, const char *, int);
int file_error(FILE *);

/* ************************************************************************* */
/* String utilities                                                          */
/* ************************************************************************* */

/* Functions returning a new string */
char *string_clone(const char *);
char *string_slice(const char *, size_t, size_t);
char *string_print(const char *, ...);
char *string_trim(const char *);
char *string_ltrim(const char *);
char *string_rtrim(const char *);
/* Functions working in place */
size_t string_length(const char *);
size_t string_lskip(const char *, int(*)(int));
size_t string_rskip(const char *, int(*)(int));

/* ************************************************************************* */
/* Date utilities                                                            */
/* ************************************************************************* */

void today(int *, int *, int *);
void now(int *, int *, int *);
void day_add(int *, int *, int *, int);
int day_of_week(int, int, int);
int ISO_day_of_week(int, int, int);
int day_of_year(int, int, int);
int week_of_month(int, int, int);
int week_of_year(int, int, int);
int month_days(int, int);
int year_is_leap(int);

#endif /* UTILS_H */

