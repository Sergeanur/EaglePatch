#pragma once

#include <Windows.h>

UINT get_private_profile_int(LPCTSTR lpKeyName, INT nDefault);

UINT get_private_profile_bool(LPCTSTR lpKeyName, INT nDefault);

DWORD get_private_profile_string(LPCTSTR lpKeyName, LPCTSTR lpDefault, LPTSTR lpReturnedString, DWORD nSize);

FLOAT get_private_profile_float(LPCTSTR lpKeyName, LPCTSTR lpDefault);

void init_private_profile();