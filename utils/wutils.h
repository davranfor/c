#ifndef WUTILS_H
#define WUTILS_H

#include <wchar.h>
#include <wctype.h>

/* ************************************************************************* */
/* String utilities                                                          */
/* ************************************************************************* */

/* Functions working on the 1st param or returning a new string if it's NULL */
char *string_wconvert(char *, const char *, wint_t (*)(wint_t));
/* Functions working inplace */
int string_wcasecmp(const char *, const char *);

#endif /* WUTILS_H */

