#pragma once

#ifndef _ENCREPT_ALBERT__
#define _ENCREPT_ALBERT__

#define MAX_PASSWD_LEN 32
#define AES_256_SIZE  256

#define DEBUG 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

bool EncreytFile(const char* oriFileName, const char* encFileName, const char* passwd);



/*
description:
	aes-cbc-256 encrept
	passwd size must >= 32 bytes
*/
bool aes_cbc_256(char* input_buf, size_t buf_size, char* passwd, char** output_ptr, size_t* saved_len, bool is_encrept = true);


#if DEBUG

// a simple hex-print routine. could be modified to print 16 bytes-per-line
void hex_print(const void* pv, size_t len);
void test(char* strs = NULL, char* user_pass=NULL);
void testFile();
#endif // !DEBUG


#endif // !_ENCREPT_ALBERT__

