#ifndef FILE_H
#define FILE_H

long file_size(FILE *);
char *file_get(FILE *, size_t);
char *file_read(const char *);

#endif /* FILE_H */

