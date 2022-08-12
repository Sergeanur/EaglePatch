#include <Windows.h>
#include <stdio.h>
#include "console.h"

FILE* conin = NULL;
FILE* conout = NULL;

void init_console()
{
	AllocConsole();

	freopen_s(&conin, "conin$", "r", stdin);
	freopen_s(&conout, "conout$", "w", stdout);
	freopen_s(&conout, "conout$", "w", stderr);
}