// mainwin.cxx
#include "mainwin.h"

// static variables /////////////////////////////////////////////////////////////////////////
// GDIresource /////////////////////////////////////////////////////////////////////////////////////////
//// GDIbrush, pen, font
COLORREF FaceColor=RGB(DLGCOLOR_R, DLGCOLOR_G, DLGCOLOR_B);
HBRUSH hbrush=NULL;
HPEN   hpen=NULL;
HFONT  hfont=NULL;

//// additional: GDIbrush
HBRUSH hbrush_dark=NULL;
HBRUSH hbrush_red=NULL;

void mainWin::RegisterClass()
{
  WNDCLASSEX wc_main={0};
  wc_main.cbSize=sizeof(WNDCLASSEX);
  wc_main.style=CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS;
  wc_main.lpfnWndProc=(WNDPROC)(mainWin::wndProcMain);
  wc_main.cbClsExtra=wc_main.cbWndExtra=0;
  wc_main.hInstance=hAppInstance;
  wc_main.hIcon=LoadIcon(hAppInstance, IDI_APPLICATION);
  wc_main.hIconSm=LoadIcon(hAppInstance, IDI_APPLICATION);
  wc_main.hCursor=LoadCursor(NULL, IDC_ARROW);
  wc_main.hbrBackground=(HBRUSH)(WHITE_BRUSH); // NULL_BRUSH, BLACKBRUSH
  wc_main.lpszMenuName=NULL; // MAKEINTRESOURCE(IDR_MENU)
  wc_main.lpszClassName=TEXT(MAINWIN_CLASS_NAME);
  if(!::RegisterClassEx(&wc_main)) PostQuitMessage(0);
}  

void mainWin::UnregisterClass()
{
  ::UnregisterClass(TEXT(MAINWIN_CLASS_NAME), hAppInstance);
}

void mainWin::RunMessageLoop()
{
  MSG msg;
  for(;;){
    if(PeekMessage(&msg, 0, 0, 0, PM_REMOVE)){
      if(msg.message==WM_QUIT){
        break;
      }else{
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
    }
  }
}

void mainWin::Show()
{
  ShowWindow(hdlg, SW_SHOW);
  ShowWindow(hwnd, SW_SHOW);
  UpdateWindow(hwnd);
}

// ConstructWindow ////////////////////////////////////////////////////////////////////////////////
mainWin::mainWin()
{
  ProcessEnd=false;
  ErrorCode=0;

  // GDIresources
  hbrush=CreateSolidBrush(FaceColor); // GetSysColor(COLOR_BTNFACE));
  hpen=CreatePen(PS_SOLID, 1, FaceColor);
  hfont=CreateFont(14, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
                   0, 0, 0, FF_DONTCARE|DEFAULT_PITCH, "Consolas");

  // GDIresource, additional
  hbrush_dark=CreateSolidBrush(RGB(0,0,0));
  hbrush_red =CreateSolidBrush(RGB(128,0,0));

  logdisp_SetCtlColor(FaceColor);

  hwnd=NULL;
  hdlg=NULL;
  hdlgLayer=NULL;
  hdlgRE=NULL;

  hthreadRE=NULL;
  hevent_initializeRE=hevent_finalizeRE=NULL;
  LoopDurationRE=1;

#ifdef USE_WPF
  // how to regist dll -> c:\Windows\Microsoft.NET\Framework\v4.0.30319\RegAsm.exe WPFpage.dll
  HRESULT hr;
  CLSID clsid;
  hr=CLSIDFromProgID(L"WPFPage.InteropTest", &clsid); // namespace.class_name
  hr=CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_IUnknown, reinterpret_cast<void **>(&pUnkWpf));
  hr=pUnkWpf->QueryInterface(IID_IDispatch, reinterpret_cast<void **>(&pDisWpf));
#endif
}

mainWin::~mainWin()
{
#ifdef USE_WPF
  SafeRelease(&pDisWpf);
  SafeRelease(&pUnkWpf);
#endif

  if(hbrush_red!=NULL)  DeleteObject(hbrush_red);
  if(hbrush_dark!=NULL) DeleteObject(hbrush_dark);

  if(hbrush!=NULL) DeleteObject(hbrush);
  if(hpen!=NULL)   DeleteObject(hpen);
  if(hfont!=NULL)  DeleteObject(hfont);

  ProcessEnd=true;
}


void mainWin::Initialize()
{
  RegisterClass();

  // get desktop size
  RECT rc_desktop;
  GetWindowRect(GetDesktopWindow(), &rc_desktop);
  int dw=rc_desktop.right-rc_desktop.left;
  int dh=rc_desktop.bottom-rc_desktop.top;

  // create_wnd
#undef FULLSCREEN // switch fullscreen
#ifdef FULLSCREEN
  hwnd=CreateWindowEx(0, TEXT(MAINWIN_CLASS_NAME), TEXT(MAINWIN_WINDOW_NAME),
                      WS_POPUP,
                      0, 0, dw, dh,
                      NULL, NULL, hAppInstance, (LPVOID)this);
#else
  hwnd=CreateWindow(TEXT(MAINWIN_CLASS_NAME), TEXT(MAINWIN_WINDOW_NAME),
                    WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX,
                    CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
                    NULL, NULL, hAppInstance, (LPVOID)this);
#endif
  SetForegroundWindow(GetDesktopWindow());
  
  // create_dlg
  hdlg=CreateDialogParam(hAppInstance, MAKEINTRESOURCE(IDD_CONTROL), hwnd,
                         (DLGPROC)dlgProcMain, (LONG_PTR)this);
#ifdef FULLSCREEN  
  SetWindowPos(hdlg, 0, 0, 0, dw, dh, SWP_NOZORDER|SWP_NOACTIVATE);
#else
  SetWindowPos(hdlg, 0, 0, 0, 0, 0, SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE);
#endif

  // create_layer
  hdlgLayer=CreateDialogParam(hAppInstance, MAKEINTRESOURCE(IDD_DIALOG_LAYER), hwnd,
                                (DLGPROC)dlgProcLayer, (LONG_PTR)this);

  // adjust wndsize
  //// Memo: useful Win32APIs for window_size adjustment...
  //// AdjustClientRect(), AdjustClientRectEx(), MapWindowPoints()
  int wx, wy, ww, wh;
#ifdef FULLSCREEN
  wx=0;  wy=0;
  ww=dw; wh=dh;
  SetWindowPos(hwnd, 0, wx, wy, ww, wh, SWP_NOZORDER| SWP_NOACTIVATE);
  SetWindowPos(hdlgLayer, 0, wx, wy, ww, wh, SWP_NOZORDER| SWP_NOACTIVATE);
#else
  RECT prc, rc;
  MyGetWindowRect(hwnd, &prc);
  wx=prc.left; wy=prc.top; // save current window position
  GetWindowRect(hdlg, &rc);
  SetRect(&prc, 0, 0, rc.right-rc.left, rc.bottom-rc.top); // set inner size
  AdjustWindowRect(&prc, GetWindowLongPtr(hwnd, GWL_STYLE), GetMenu(hwnd)? TRUE : FALSE); // adjust window size
  ww=prc.right-prc.left, wh=prc.bottom-prc.top;
  SetWindowPos(hwnd, 0, wx, wy, ww, wh, SWP_NOZORDER| SWP_NOACTIVATE);

  GetClientRect(hdlg, &rc);
  POINT point;
  point.x=rc.left;
  point.y=rc.top;
  MapWindowPoints(hdlg, NULL, &point, 1); // num of points=1
  SetWindowPos(hdlgLayer, HWND_TOPMOST, point.x, point.y, rc.right-rc.left, rc.bottom-rc.top, SWP_NOACTIVATE);
#endif

  // create_threads in sequence
  hevent_initializeRE=CreateEvent(NULL, FALSE, FALSE, NULL);
  hevent_finalizeRE=CreateEvent(NULL, FALSE, FALSE, NULL);
  hthreadRE=CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)thrProcRE, (PVOID *)this, 0, NULL);
  WaitForSingleObject(hevent_initializeRE, 10000);
  CloseHandle(hevent_initializeRE); hevent_initializeRE=NULL;
}

void mainWin::Uninitialize()
{
  UnregisterClass();
}


void mainWin::TimerFunc(WPARAM id)
{
  static __time32_t appstarttime=0;
  static __time32_t winlogontime=0;
  static bool doonce=false;
  
  if(id==ONCE_TIMERID){
    // get appstarttime
    if(appstarttime==0) appstarttime=_time32(NULL);
    
    // get winstarttime
    if(winlogontime==0){
      WCHAR UserName[256];
      DWORD nUserName=sizeof(UserName);
      BOOL bret=GetUserNameW(UserName, &nUserName);
      if(bret==TRUE){
        DWORD dwLevel=2;
        LPUSER_INFO_0 pBuf=NULL;
        NET_API_STATUS nStatus=NetUserGetInfo(NULL, UserName, dwLevel, (LPBYTE *)&pBuf);
        if(nStatus==NERR_Success){
          winlogontime=((LPUSER_INFO_2)pBuf)->usri2_last_logon;
        }
      }
    }
    
    if((_time32(NULL)-winlogontime<10)||(_time32(NULL)-appstarttime<2)){
      if(doonce==false){
        // do once    
        ShowWindowAsync(hdlgRE, SW_SHOWNA);
        logdisp_PrintTextf("Waiting...");
        doonce=true;
      }
      return;
    }

    logdisp_PrintTextf("System is available now.\n");
    SetForegroundWindow(hwnd);

    // clean msg from messageloop
    MSG msg;
    while(PeekMessage(&msg, 0, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE));
    KillTimer(hwnd, ONCE_TIMERID);

    // hide layer
    ShowWindow(hdlgLayer, SW_HIDE);
  }
}

bool GetGDIBITMAPfromResource(HBITMAP *hbmp, int *w_, int *h_, int *b_, LPCTSTR pResourceType, UINT pResourceID)
{
  *hbmp=NULL;
  *w_=*h_=*b_=0;
  LPCTSTR pResourceName=MAKEINTRESOURCE(pResourceID);
  HRSRC hResource=NULL;
  if(pResourceType==RT_BITMAP){
    *hbmp=LoadBitmap(hAppInstance, pResourceName);
    goto extractsize;
  }else{
    hResource=FindResource(hAppInstance, pResourceName, pResourceType);
  }
  if(hResource==NULL) return false;

  DWORD dwResourceSize=SizeofResource(hAppInstance, hResource);
  unsigned char *pResourceData=(unsigned char *)LockResource(LoadResource(hAppInstance, hResource));

  IStream *pIStream=SHCreateMemStream(pResourceData, dwResourceSize);
  {
    ULONG_PTR gdiplus_token;
    GdiplusStartupInput gsi;
    GdiplusStartup(&gdiplus_token, &gsi, NULL);
    {
      Gdiplus::Bitmap *pGDIplusBitmap=Gdiplus::Bitmap::FromStream(pIStream);
      pGDIplusBitmap->GetHBITMAP(NULL, hbmp);
      delete pGDIplusBitmap;
    }
    GdiplusShutdown(gdiplus_token);
  }
  pIStream->Release();

 extractsize:
  BITMAP bm;
  GetObject(*hbmp, sizeof(BITMAP), &bm);
  *w_=bm.bmWidth;
  *h_=bm.bmHeight;
  *b_=bm.bmBitsPixel;
  return true;
}

LRESULT CALLBACK mainWin::wndProcMain(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  static mainWin *_this=NULL;

  switch(msg){
  case WM_CREATE:
    _this=reinterpret_cast<mainWin *>(reinterpret_cast<CREATESTRUCT *>(lparam)->lpCreateParams);
    SetTimer(hwnd, ONCE_TIMERID, 1000, NULL); // start 1000msec after
    break;

  case WM_CLOSE:
    DestroyWindow(hwnd);
    break;

  case WM_DESTROY:
    {
      // terminate threads in sequence, timeout=10000msec
      int timeout=10000;
      SetEvent(_this->hevent_finalizeRE);
      WaitForSingleObject(_this->hthreadRE, timeout);
      CloseHandle(_this->hthreadRE);
      CloseHandle(_this->hevent_finalizeRE);

      if(_this->ProcessEnd==false){
        _this->ProcessEnd=true;
      }
      PostQuitMessage(0);
    }
    break;

  case WM_MOVE:
    {
      RECT rc_dlg, rc_follow;
      POINT point;

      GetWindowRect(_this->hdlg, &rc_dlg);
      GetWindowRect(GetDlgItem(_this->hdlg, IDC_STATIC_RE), &rc_follow);
      point.x=rc_follow.left-rc_dlg.left;
      point.y=rc_follow.top-rc_dlg.top;
      MapWindowPoints(_this->hdlg, NULL, &point, 1); // num of points=1
      SetWindowPos(_this->hdlgRE, _this->hdlgLayer, point.x, point.y, 0, 0, SWP_NOSIZE|SWP_NOACTIVATE);

      SetFocus(_this->hdlg);
    }
    break;

  case WM_TIMER:
    {
      _this->TimerFunc(wparam);
    }
    break;

  default:
    return DefWindowProc(hwnd, msg, wparam, lparam);
  }
  return 0;
}

void mainWin::CommandFunc(WPARAM wparam, LPARAM lparam)
{
  switch(LOWORD(wparam)){
  case IDC_BUTTON_TEST1:
    {
      std::random_device rnd;
      std::mt19937 mt(rnd());
      float random_number=(float)mt()/(float)0xffffffff; // 0~1
      
      int minimum=0, maximum=100;
      float rndval=(float)minimum+(float)(maximum-minimum)*random_number;
      
      logdisp_PrintTextf("RandomValue=%f", rndval);
      return;
    }
    break;

  case IDC_BUTTON_TEST2:
    {
      HBITMAP hbmp;
      int w, h, b;

      GetGDIBITMAPfromResource(&hbmp, &w, &h, &b, "PNG", IDR_PNG_BLUEBALL);
      PostMessage(hdlgRE, WM_COMMAND, ID_INSERT_HBMP_AND_DELETE, (LPARAM)hbmp);

    }
    break;

  case IDC_BUTTON_TEST3:
    {
      logdisp_SetTextColor(RGB(255,0,0));
      logdisp_PrintTextf("Red");
      logdisp_SetTextColor(RGB(0,0,0));
    }
    break;

  case IDC_BUTTON_UNCHECK:
    {
#ifdef USE_WPF
      WpfFunc();
#endif
    }
    break;

  case IDC_BUTTON_MINIMIZE:
    {
      ShowWindow(hwnd, SW_MINIMIZE);
    }
    break;

  case IDC_BUTTON_QUIT:
    {
      DestroyWindow(hwnd);
    }
    break;
  }
  return;
}

LRESULT CALLBACK mainWin::dlgProcMain(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
  static mainWin *_this=NULL;
  switch(msg){
  case WM_INITDIALOG:
    {
      if(_this==NULL) _this=reinterpret_cast<mainWin *>(lparam);
#ifdef USE_WPF
      RECT rc_wpf;
      GetWindowRect(GetDlgItem(hdlg, IDC_STATIC_WPF), &rc_wpf);

      std::vector<POINT> p(1);
      p[0].x=rc_wpf.left;
      p[0].y=rc_wpf.top;
      MapWindowPoints(NULL, hdlg, &p[0], p.size());

      int width=rc_wpf.right-rc_wpf.left;
      int height=rc_wpf.bottom-rc_wpf.top;
      _this->hdlgWpf=ManagedCode::StickWPFPage(hdlg, p[0].x, p[0].y, width, height);
#endif
      return FALSE; // if FALSE, avoid focusing component part
    }
    break;
  case WM_PAINT:
    {
      HRGN hrgn=NULL;
      HDC hdc=GetDC(hdlg);
      if(GetClipRgn(hdc, hrgn)!=1){
        RECT cliprc;
        GetClientRect(_this->hdlgRE, &cliprc);
        hrgn=CreateRectRgnIndirect(&cliprc);
        ExtSelectClipRgn(hdc, hrgn, RGN_DIFF);
        DeleteObject(hrgn);
      }
      ReleaseDC(hdlg, hdc);
    }
    break;
  case WM_CTLCOLORSTATIC:
  case WM_CTLCOLORDLG:
    {
      HDC hDC=(HDC)wparam; HWND hCtrl=(HWND)lparam;
      return (LRESULT)hbrush;
    }
    break;
  case WM_COMMAND:
    {
      _this->CommandFunc(wparam, lparam);
    }
    break;
  }
  return FALSE; // dialog, if wish to do nothing subsequently, return 0(FALSE)
}

LRESULT CALLBACK mainWin::dlgProcLayer(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
  static mainWin *_this=NULL;
  switch(msg){
  case WM_INITDIALOG:
    {
      if(_this==NULL) _this=reinterpret_cast<mainWin *>(lparam);
      SetWindowLongPtr(hdlg, GWL_EXSTYLE, GetWindowLongPtr(hdlg, GWL_EXSTYLE)|WS_EX_LAYERED);
      SetLayeredWindowAttributes(hdlg, 0, (unsigned char)(50*255/100), LWA_ALPHA);
      ShowWindow(hdlg, SW_HIDE);
      SetFocus(_this->hdlg);
      return FALSE; // if FALSE, avoid focusing component part
    }
    break;
  case WM_CTLCOLORDLG:
    {
      HDC hDC=(HDC)wparam; HWND hCtrl=(HWND)lparam;
      if(hCtrl==hdlg){
        return (_this->ErrorCode==0) ? (LRESULT)hbrush_dark : (LRESULT)hbrush_red;
      }
    }
    break;
  case WM_MOUSEACTIVATE:
    {
      SetWindowLongPtr(hdlg, DWLP_MSGRESULT, MA_NOACTIVATE);
      return TRUE;
    }
    break;
  }
  return FALSE;
}
