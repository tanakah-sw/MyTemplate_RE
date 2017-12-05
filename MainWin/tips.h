// tips.h

// textformat
#define CHAR_LF   "\n"      // 0x0A
#define CHAR_CR   "\r"      // 0x0D
#define CHAR_CRLF "\r\n"
#define WCHAR_LF   L"\n"      // 0x0A
#define WCHAR_CR   L"\r"      // 0x0D
#define WCHAR_CRLF L"\r\n"
static const float HIMETRIC_INCH=2540.f;
static const float MONITOR_DPI=96.f;

// IA32
#define BREAKPOINT __asm int 3

// DWM -------------------------------------------------------------------------------------
#ifndef DWMWA_EXTENDED_FRAME_BOUNDS
#define DWMWA_EXTENDED_FRAME_BOUNDS 0x09
#endif

inline void MyGetWindowRect(HWND hwnd, RECT *rect)
{
  static HRESULT (WINAPI *DwmIsCompositionEnabled)(BOOL *)=NULL;
  static HRESULT (WINAPI *DwmGetWindowAttribute)(HWND, DWORD, LPCVOID, DWORD)=NULL;
  BOOL bAero=FALSE;

  if((DwmIsCompositionEnabled==NULL)||(DwmGetWindowAttribute==NULL)){
    HMODULE hdll=LoadLibrary("dwmapi");
    if(hdll){
      DwmIsCompositionEnabled=(HRESULT(WINAPI *)(BOOL *))
        GetProcAddress(hdll, "DwmIsCompositionEnabled");  
      DwmGetWindowAttribute=(HRESULT(WINAPI *)(HWND, DWORD, LPCVOID, DWORD))
        GetProcAddress(hdll, "DwmGetWindowAttribute");
    }
  }
  
  if(DwmIsCompositionEnabled!=NULL){
    DwmIsCompositionEnabled(&bAero);
  }
  if(bAero==FALSE){
    GetWindowRect(hwnd, rect);
    return;
  }else{
    if(DwmGetWindowAttribute!=NULL){
      DwmGetWindowAttribute(hwnd, DWMWA_EXTENDED_FRAME_BOUNDS, rect, sizeof(RECT));
    }
    return;
  }
}
