// wpfclr.cxx
#include "mainwin.h"

#ifdef USE_WPF
bool mainWin::WpfFunc()
{
  HRESULT hr;

  LPOLESTR funcname=L"UnCheck";
  DISPID dispid=0;
  hr=pDisWpf->GetIDsOfNames(IID_NULL, &funcname, 1, LOCALE_SYSTEM_DEFAULT, &dispid);
  if(FAILED(hr)) return false;

  DISPPARAMS params;
  memset(&params, 0, sizeof(DISPPARAMS));
  params.cArgs=0;
  params.cNamedArgs=0;
  params.rgdispidNamedArgs=NULL;
  params.rgvarg = NULL;
  
  VARIANT vRet;
  VariantInit(&vRet);
  
  hr=pDisWpf->Invoke(dispid, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_METHOD, &params, &vRet, NULL, NULL);
  if(FAILED(hr)) return false;
  return true;
}
#endif

#ifdef USE_WPF
namespace ManagedCode
{
  using namespace System;
  using namespace System::Windows;
  using namespace System::Windows::Interop;

  static HWND p_hwnd=NULL;
  
  IntPtr ChildHwndSourceHook(IntPtr, int msg, IntPtr wparam, IntPtr lparam, bool% handled)
  {
    if(msg==WM_COMMAND){
      ATOM a=(ATOM)((int)lparam);
      wchar_t wbuf[256];
      GetAtomNameW(a, wbuf, 256);
      DeleteAtom(a);

      char buf[256];
      WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)wbuf, -1, buf, 256, NULL, NULL);

      SetDlgItemText(p_hwnd, IDC_STATIC_MESSAGE, buf);
    }
    return IntPtr(0);
  }

  HWND StickWPFPage(HWND parent, int x, int y, int width, int height)
  {
    p_hwnd=parent;
    HwndSourceParameters params;
    params.PositionX=x;
    params.PositionY=y;
    params.Width=width;
    params.Height=height;
    params.ParentWindow=IntPtr(parent);
    params.WindowStyle=WS_VISIBLE| WS_CHILD;
    
    HwndSource^ source=gcnew HwndSource(params);
    source->AddHook(gcnew HwndSourceHook(ChildHwndSourceHook));

    UIElement^ page=gcnew WPFPage::TestPanel(true);
    source->RootVisual=page;

    return (HWND)source->Handle.ToPointer();
  }
}
#endif
