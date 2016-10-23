#pragma once
#include <stdio.h>
#include <memory.h>
#include <time.h>
#include <stdlib.h>

#define PLAIN_FILE_OPEN_ERROR -1
#define KEY_FILE_OPEN_ERROR -2
#define CIPHER_FILE_OPEN_ERROR -3
#define OK 1

typedef char ElemType;


int ISD_DES_Encrypt(char *plainFile, char *keyStr, char *cipherFile);
int ISD_DES_Decrypt(char *cipherFile, char *keyStr, char *plainFile);