// mainwin.h
#include "config.h"
#include "common.h"

// optional headers /////////////////////////////////////////////////////////////////////////
// COM
#ifdef USE_COM
#include <comutil.h>
#include <objbase.h>
#pragma comment(lib, "comsupp.lib")
#pragma comment(lib, "comsuppw.lib")
#endif

#ifdef USE_WRL
#ifndef __cplusplus_cli
#include <shobjidl.h>
#include <wrl\client.h>
#include <wrl\implements.h>
#endif
#endif

#ifdef USE_GDIPLUS
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
using namespace Gdiplus;
#endif

// my-utilities //////////////////////////////////////////////////////////////////////////////////
#include "tips.h"
#include "logging.h"

// implement in wpfclr.cxx
#ifdef USE_WPF
namespace ManagedCode{
extern HWND StickWPFPage(HWND parent, int x, int y, int width, int height);
}
#endif

// constant-defines
#include "resource.h"

#define MAINWIN_CLASS_NAME  "mymainwinclass"
#define MAINWIN_WINDOW_NAME "mymainwin"

#define ID_INSERT_HBMP_AND_DELETE 0

#define ONCE_TIMERID 1
#define SEQU_TIMERID 2

// dialog color
#define DLGCOLOR_R 250
#define DLGCOLOR_G 250
#define DLGCOLOR_B 250


class mainWin
{
public:
  mainWin();
  ~mainWin();
  void Initialize();
  void Show();
  void RunMessageLoop();
  void Uninitialize();

private:
  void TimerFunc(WPARAM id);
  void CommandFunc(WPARAM wparam, LPARAM lparam);

  // windows
  HWND hwnd;
  HWND hdlg;
  HWND hdlgRE;
  HWND hdlgWpf;
  HWND hdlgLayer;

  // basewindow
  static LRESULT CALLBACK wndProcMain(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

  // dialogwindow
  static LRESULT CALLBACK dlgProcMain(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam);
  static LRESULT CALLBACK dlgProcLayer(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam);

  // threads
  //// update richedit independently from main_messageloop
  static DWORD WINAPI thrProcRE(void *);
  HANDLE hthreadRE;
  HANDLE hevent_initializeRE;
  HANDLE hevent_finalizeRE;
  DWORD  LoopDurationRE;

  void RegisterClass();
  void UnregisterClass();
  
#ifdef USE_WPF
  IDispatch *pDisWpf;
  IUnknown  *pUnkWpf;
  bool WpfFunc();
#endif

  bool ProcessEnd;
  WORD ErrorCode;
};

// GDIresource /////////////////////////////////////////////////////////////////////////////////////////
//// GDIbrush
extern HBRUSH hbrush;
//// GDIfont
extern HFONT  hfont;
//// GDIpen
extern HPEN   hpen;

//// GDIbrush
extern HBRUSH hbrush_dark;
extern HBRUSH hbrush_red;
