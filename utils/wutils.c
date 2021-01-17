#include <stdlib.h>
#include <string.h>
#include "wutils.h"

char *string_wconvert(char *ptr, const char *str, wint_t (*func)(wint_t))
{
    if (mbtowc(NULL, 0, 0) == -1)
    {
        return NULL;
    }

    size_t len = strlen(str);
    const char *end = str + len;
    char *new = NULL;

    if (ptr == NULL)
    {
        ptr = new = malloc(len + 1);
        if (ptr == NULL)
        {
            return NULL;
        }
    }

    char *temp = ptr;
    char mb[4];
    wchar_t wc;
    int size;

    while ((size = mbtowc(&wc, str, (size_t)(end - str))) > 0)
    {
        str += size;
        wc = (wchar_t)func((wint_t)wc);
        size = wctomb(mb, wc);
        memcpy(ptr, mb, (size_t)size);
        ptr += size;
    }
    *ptr = '\0';
    if (size == -1)
    {
        if (new != NULL)
        {
            free(new);
        }
        return NULL;
    }
    return temp;
}

int string_wcasecmp(const char *str1, const char *str2)
{
    const char *end1 = str1 + strlen(str1);
    const char *end2 = str2 + strlen(str2);

    mbtowc(NULL, 0, 0);
    for (;;)
    {
        wchar_t wc1;
        wchar_t wc2;
        int size1 = mbtowc(&wc1, str1, (size_t)(end1 - str1));
        int size2 = mbtowc(&wc2, str2, (size_t)(end2 - str2));

        if (size1 != size2)
        {
            return size1 < size2 ? -1 : +1;
        }
        wc1 = (wchar_t)towlower((wint_t)wc1);
        wc2 = (wchar_t)towlower((wint_t)wc2);
        if (wc1 != wc2)
        {
            return wc1 < wc2 ? -1 : +1;
        }
        if (wc1 == 0)
        {
            return 0;
        }
        str1 += size1;
        str2 += size2;
    }
}

