#pragma once
#ifndef  _PATCHES_ALBERT__
#define _PATCHES_ALBERT__
#include <Windows.h>

#define ArraySize(ptr)	(sizeof(ptr) / sizeof(ptr[0]))

void InitDir(bool hide);
bool StartHookService();

#endif // ! _PATCHES_ALBERT__
