#pragma once

#ifndef _ENCREPT_ALBERT__
#define _ENCREPT_ALBERT__

#define MAX_PASSWD_LEN 32
#define AES_256_SIZE  256

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

bool EncreptFile(const char* oriFileName, const char* encFileName, const char* passwd);



#endif // !_ENCREPT_ALBERT__

