#ifndef _MSC_VER
#error This code can be only compiled using Visual Studio
#endif

#pragma once

#include <WinSock2.h>
#include <plInterface.h>

#ifdef _DEBUG
# define CRTDBG_MAP_ALLOC
# include <stdlib.h>
# include <crtdbg.h>
#endif

#define _CRT_SECURE_NO_DEPRECATE 1
#define _CRT_SECURE_NO_WARNINGS 1
#define _ATL_SECURE_NO_DEPRECATE 1
#define _CRT_NON_CONFORMING_SWPRINTFS 1

#include <windows.h>
#include <commctrl.h>
#include <richedit.h>
#include <iostream>
#include <time.h>
#include <vector>
#include <stack>
#include <sstream>
#include <algorithm>
#include "../common/StringUtils.h"
#include "libs/include/sqlite3.h"
#include "utlIgnore.h"
#include "resource.h"

#pragma comment (lib,"Comctl32.lib")

#ifdef _M_IX86
 #pragma comment (lib,"../API/libs/lib/libSQ3_Release_Win32_libSQ3.lib")
#elif _M_X64
 #pragma comment (lib,"../API/libs/lib/libSQ3_Release_x64_libSQ3.lib")
#else
 #error Unknown arch
#endif

static const wchar_t MDL[] = L"ARCH";
