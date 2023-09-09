#pragma once

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cctype>
#include <list>
#include <string>

#include "asm.h"
#include "asmops.h"
#include "asmexpr.h"

#ifdef _LINUX
#define _stricmp strcasecmp
#define _strnicmp strncasecmp
#define _countof(a) (sizeof(a)/sizeof(*(a)))

inline char* _strupr(char* str)
{
    while (*str != '\0')
    {
        *str = toupper(*str);
        str++;
    }
    return str;
}
#endif
