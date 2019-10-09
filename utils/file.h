#ifndef FILE_H
#define FILE_H

long fgetsize(FILE *);
char *fgettext(FILE *);
size_t fsettext(FILE *, const char *);

FILE *file_open(const char *, const char *);
long file_get_size(const char *);
char *file_read(const char *);
size_t file_write(const char *, const char *, int);

#endif /* FILE_H */

