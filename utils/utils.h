#ifndef UTILS_H
#define UTILS_H

FILE *file_open(const char *, const char *);
long file_get_size(const char *);
char *file_read(const char *);
size_t file_write(const char *, const char *, int);

long fgetsize(FILE *);
char *fgettext(FILE *);
size_t fsettext(FILE *, const char *);
char *fgetline(FILE *);

char *dupstr(const char *);
char *dupnstr(const char *, size_t);
char *dupstrf(const char *, ...);

char *trim(const char *);
char *ltrim(const char *);
char *rtrim(const char *);
void trim_inplace(char *);
void ltrim_inplace(char *);
void rtrim_inplace(char *);

#endif /* UTILS_H */

