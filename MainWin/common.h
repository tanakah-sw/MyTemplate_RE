// compiler options /////////////////////////////////////////////////////////////////////////
#pragma once
#pragma warning(disable: 4100) // disable warning "unreferenced formal parameter"
#pragma warning(disable: 4102) // disable warning "unreferenced find_rule label"
#pragma warning(disable: 4189) // disable warning "local variable is initialized but"
#pragma warning(disable: 4458) // disable warning "declaration hides class member"
#pragma warning(disable: 4996) // disable warning "may be unsafe"
#pragma warning(disable: 4611) // disable warning "interaction between '_setjmp' and C++ object destruction is non-portable"
#pragma warning(disable: 4091) // disable warning "'typedef': ignored on left of 'tagGPFIDL_FLAGS'
#pragma warning(disable: 4505) // disable warning "Unreferenced local function has been removed"
#pragma warning(disable: 4530) // disable warning "C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc"

#define _HAS_EXCEPTIONS 0      // disable warning 4577 "'noexcept' used with no exception handling mode specified; Specify /EHsc"

// common defines
#define MAX_NUMERICSTRING 16
#define MAX_SHORTSTRING   64
#define MAX_LONGSTRINGS   256

// basic headers ///////////////////////////////////////////////////////////////////////////
// CRT
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#define _USE_MATH_DEFINES // use M_PI
#include <math.h>
#include <time.h>
#include <sys/types.h>
#include <sys/timeb.h>

// STL
#include <array>
#include <vector>
#include <tuple>
#include <map>
#include <unordered_map>
#include <queue>
#include <functional>
#include <random>

// WindowsAPI
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600 // WindowsVersion: WindowsVista/WindowsServer2008
#endif

#define WIN32_LEAN_AND_MEAN
#define OEMRESOURCE

// Winsock
#include <winsock2.h>
#pragma comment(lib , "ws2_32.lib")

#include <windows.h>
#include <shlobj.h>
#include <shellapi.h>

#pragma comment (lib, "mpr.lib") // multiple provider router

// GDI/Alpha
#pragma comment(lib, "msimg32.lib")

// UserSecurity
#include <lm.h>
#include <sddl.h>
#pragma comment (lib, "netapi32.lib")

#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

#include <uxtheme.h>
#pragma comment(lib, "uxtheme.lib")

#include <commdlg.h>
#pragma comment(lib, "comdlg32.lib")

#include <commctrl.h>
#pragma comment(lib, "comctl32.lib")
#pragma comment(linker, "\"/manifestdependency:type='Win32' "\
    "name='Microsoft.Windows.Common-Controls' version='6.0.0.0' "\
    "processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// Direct2D
#include <d2d1.h>
#include <d2d1helper.h>
#pragma comment(lib, "d2d1.lib")

//// DirectWrite
#include <dwrite.h>
#pragma comment(lib, "dwrite.lib")

//// WIC
#include <wincodec.h>
#pragma comment(lib, "windowscodecs.lib")

// global_variables
extern HINSTANCE hAppInstance;
extern char HomeFolderName[MAX_PATH];
extern CRITICAL_SECTION c_section;

extern ID2D1Factory          *pD2D_Factory;
extern IWICImagingFactory    *pWIC_Factory;
extern IDWriteFactory        *pDWR_Factory;

// image_tips
#define MAX_SIZEOFBITMAPINFO ((sizeof(BITMAPINFOHEADER)+sizeof(RGBQUAD))+sizeof(RGBQUAD)*256)
inline int BytesPerLine(int width, int bitcount)
{
  return (width*((bitcount+7)/8*8)+31)/32*4; // 4bytes=32bits align
}

// com_tips
template<class Interface>
inline void SafeRelease(Interface **ppInterfaceToRelease)
{
  if(*ppInterfaceToRelease!=NULL){
    (*ppInterfaceToRelease)->Release();
    (*ppInterfaceToRelease)=NULL;
  }
}

