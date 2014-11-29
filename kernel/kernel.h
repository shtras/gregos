#pragma once
#include "defs.h"

void panic(const char* errMsg = NULL);

#define Q(x) #x
#define QUOTE(x) Q(x)

#define kassert(expr) (void)((!!(expr)) || (panic("Assertion failed: " QUOTE(expr) " at " __FILE__ ":" QUOTE(__LINE__)), 0));
