#ifndef UTILS_H
#define UTILS_H

/* File utilities */
long file_get_size(const char *);
char *file_read(const char *);
size_t file_write(const char *, const char *, int);
char *file_get_line(FILE *);

/* String utilities */
char *string_clone(const char *);
char *string_slice(const char *, size_t, size_t);
char *string_print(const char *, ...);
char *string_trim(const char *);
char *string_ltrim(const char *);
char *string_rtrim(const char *);
void string_trim_inplace(char *);
void string_ltrim_inplace(char *);
void string_rtrim_inplace(char *);

#endif /* UTILS_H */

