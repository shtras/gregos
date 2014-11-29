#include "string.h"

int strlen(const char* str)
{
    int res = 0;
    while (++res, *str++);
    return res-1;
}
