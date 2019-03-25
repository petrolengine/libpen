#include <string.h>
#include <ctype.h>

#include "string_utils.h"


char *
pen_strip(char *str)
{
    char *p = str;
    size_t l;

    while ((isspace(*p)))
        p++;
    l = strlen(p);
    if (p != str)
        memmove(str, p, l + 1);

    if (l) {
        p = str + l - 1;
        while ((isspace(*p)))
            *p-- = 0;
    }
    return str;
}
