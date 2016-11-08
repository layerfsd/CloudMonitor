#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/aes.h>
#include <openssl/rand.h>

#include <Windows.h>
#include <iostream>
#include "encrept.h"

using namespace std;

//learn from
//http://stackoverflow.com/questions/18152913/aes-aes-cbc-128-aes-cbc-192-aes-cbc-256-encryption-decryption-with-openssl-c

#pragma comment(lib,"libcrypto.lib")	// ssl 加密函数

// a simple hex-print routine. could be modified to print 16 bytes-per-line
void hex_print(const void* pv, size_t len)
{
	const unsigned char * p = (const unsigned char*)pv;
	if (NULL == pv)
	{
		printf("NULL");
		return;
	}

	size_t i = 0;
	for (; i < len; ++i)
	{
		if (i > 0  && 0 == i % 18)
		{
			printf("\n");
		}
		printf("%02X ", *p++);
	}
	printf("\n");

	return;
}

bool aes_cbc_256(char* input_buf, size_t buf_size, char* passwd, char** output_ptr, size_t* saved_len, bool is_encrept)
{
	size_t encslength;
	if (is_encrept)
	{
		encslength = ((buf_size + AES_BLOCK_SIZE) / AES_BLOCK_SIZE) * AES_BLOCK_SIZE;
	}
	else
	{
		encslength = buf_size;
	}

	*saved_len = encslength;

	unsigned char  aes_key[AES_256_SIZE / 8];
	unsigned char  IV[AES_BLOCK_SIZE];
	unsigned char* output = (unsigned char*)malloc(encslength);

	int passwd_len = strlen(passwd);

	if (NULL == input_buf) return false;
	if (NULL == output) return false;
	if (passwd_len < 0 || passwd_len >= MAX_PASSWD_LEN) return false;

	memset(IV, 0, sizeof(IV));
	memset(aes_key, 0, sizeof(aes_key));
	memset(output, 0, encslength);

	memcpy(aes_key, passwd, MAX_PASSWD_LEN);

	AES_KEY aes_ctx_key;
	if (is_encrept)
	{
		AES_set_encrypt_key(aes_key, AES_256_SIZE, &aes_ctx_key);
		AES_cbc_encrypt((unsigned char*)input_buf, output, buf_size, &aes_ctx_key, IV, AES_ENCRYPT);
	}
	else
	{
		AES_set_decrypt_key(aes_key, AES_256_SIZE, &aes_ctx_key);
		AES_cbc_encrypt((unsigned char*)input_buf, output, encslength, &aes_ctx_key, IV, AES_DECRYPT);
	}

	*output_ptr = (char *)output;

	return true;
}


// main entrypoint
int testAES()
{
	/* generate input with a given length */
	unsigned char aes_input[] = "Hello Winter.";
	const size_t inputslength = sizeof(aes_input);

	// buffers for encryption and decryption
	const size_t encslength = ((inputslength + AES_BLOCK_SIZE) / AES_BLOCK_SIZE) * AES_BLOCK_SIZE;

	unsigned char IV[AES_BLOCK_SIZE] = { 0 };


	/* init vector */
	unsigned char enc_out[encslength];
	unsigned char dec_out[encslength];

	memset(enc_out, 0, sizeof(enc_out));
	memset(dec_out, 0, sizeof(dec_out));


	/* generate a key with a given length */
	unsigned char aes_key[AES_256_SIZE / 8] = "ILoveYou";

	// so i can do with this aes-cbc-128 aes-cbc-192 aes-cbc-256
	AES_KEY enc_key, dec_key;
	AES_set_encrypt_key(aes_key, AES_256_SIZE, &enc_key);
	AES_cbc_encrypt(aes_input, enc_out, inputslength, &enc_key, IV, AES_ENCRYPT);

	memset(IV, 0, sizeof(IV));

	AES_set_decrypt_key(aes_key, AES_256_SIZE, &dec_key);
	AES_cbc_encrypt(enc_out, dec_out, encslength, &dec_key, IV, AES_DECRYPT);


	printf("original:\n");
	printf("%s\n", aes_input);
	hex_print(aes_input, sizeof(aes_input));


	printf("encrypt:\n");
	hex_print(enc_out, sizeof(enc_out));

	printf("decrypt:\n");
	printf("%s\n", dec_out);
	hex_print(dec_out, sizeof(aes_input));

	return 0;
}

inline void InitDir()
{

	char strModule[MAX_PATH];
	GetModuleFileName(NULL, strModule, MAX_PATH); //得到当前模块路径
	strcat(strModule, "//..//");     //设置为当前工作路径为当时的上一级
	SetCurrentDirectory(strModule);
	GetCurrentDirectory(sizeof(strModule), strModule);

	cout << "Working Dir: " << strModule << endl;
	return;
}



// main entrypoint
int main(int argc, char **argv)
{
	//testAES();
	//testEncrept();
	//test();
	InitDir();
	testFile();
	return 0;
}