// logging.cxx
#include "config.h"
#include "common.h"

#include "logging.h"
#include "tips.h"

#include "richeditole.h"

static HWND OutputWnd=NULL;
static void logdisp_WriteWnd(char MText[]);
static void logdisp_WriteDbg(char MText[]);

const int maxchars=1024;
const int maxlines=32000;
static HBRUSH hbrush=NULL;

void logdisp_PrintTextf(char *fmt, ...)
{
  char MText[1024]={0};
  char **lppParam;
  lppParam=((char **)&fmt)+1;

  EnterCriticalSection(&c_section);
  vsprintf(MText, fmt, (va_list)lppParam);
  if(OutputWnd==NULL){
    logdisp_WriteDbg(MText);
  }else{
    logdisp_WriteWnd(MText);
  }
  LeaveCriticalSection(&c_section);
}

void logdisp_DrawBitmap(HBITMAP hbmp, int w, int h)
{
  if(OutputWnd!=NULL) InsertHBITMAP(OutputWnd, hbmp, w, h);
}

static LRESULT CALLBACK RESubclassProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam,
                                       UINT_PTR, DWORD_PTR)
{
  switch(msg){
  case WM_LBUTTONDBLCLK:
    {
      HBITMAP hbmp;
      int w, h;
      extractHBITMAP(OutputWnd, &hbmp, &w, &h);
      logdisp_PrintTextf("dblclk image");
    }
    break;
  case WM_COMMAND:
    {
      switch(LOWORD(wparam)){
      case IDM_RE_OPEN:
        {
          HBITMAP hbmp;
          int w, h;
          extractHBITMAP(OutputWnd, &hbmp, &w, &h);
          logdisp_PrintTextf("select menu OPEN");
        }
        break;
      case IDM_RE_SAVE:
        {
          HBITMAP hbmp;
          int w, h;
          extractHBITMAP(OutputWnd, &hbmp, &w, &h);
          logdisp_PrintTextf("select menu SAVE");
        }
        break;
      }
    }
    break;
  }
  return DefSubclassProc(hwnd, msg, wparam, lparam);
}

void logdisp_SetCtlColor(COLORREF cr)
{
  hbrush=CreateSolidBrush(cr);
}

LRESULT CALLBACK logdisp_dlgProcRE(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
  static DWORD CTL_ID=0;
  static HWND hwndRE=NULL;
  static IExRichEditOleCallback *pREOLECallback=NULL;
  
  switch(msg){
  case WM_INITDIALOG:
    {
      CTL_ID=(DWORD)lparam;
      if(CTL_ID!=0){
        hwndRE=GetDlgItem(hdlg, CTL_ID);
        logdisp_SetOutputWnd(hwndRE);
        SetWindowSubclass(hwndRE, RESubclassProc, 0, 0);
        pREOLECallback=new IExRichEditOleCallback;
        SendMessage(hwndRE, EM_SETOLECALLBACK, 0, (LPARAM)pREOLECallback);
        pREOLECallback->SetHWND(hwndRE);
      }
      ShowWindow(hdlg, SW_HIDE);
      return FALSE; // if FALSE, avoid focusing component
    }
    break;
  case WM_DESTROY:
    {
      if(hbrush!=NULL) DeleteObject(hbrush);
    }
    break;
  case WM_SHOWWINDOW:
    {
      if(wparam==TRUE){                                // being shown
        UpdateWindow(GetAncestor(hdlg, GA_ROOTOWNER)); // update preceding
        Sleep(300);                                    // show, 300ms delayed
      }
    }
    break;
  case WM_CTLCOLORSTATIC:
  case WM_CTLCOLORDLG:
    {
      if(hbrush!=NULL){
        HDC hDC=(HDC)wparam; HWND hCtrl=(HWND)lparam;
        return (LRESULT)hbrush;
      }
    }
    break;
  }
  return FALSE;
}

void logdisp_SetOutputWnd(HWND hwnd)
{
  OutputWnd=hwnd;
}

void logdisp_WriteDbg(char MText[])
{
  OutputDebugString(MText);
  OutputDebugString(CHAR_LF);
}

void logdisp_WriteWnd(char MText[])
{
  static bool doonce=false;
  SendMessage(OutputWnd, WM_SETREDRAW, FALSE, 0);
  {
    char linebuf[maxchars];
    int lines=(int)SendMessage(OutputWnd, EM_GETLINECOUNT, 0, 0);
    if(lines>maxlines){
      *(LPDWORD)linebuf=sizeof(linebuf)/sizeof(char);
      int charlen=(int)SendMessage(OutputWnd, EM_GETLINE, 0, (LPARAM)linebuf);
      if(charlen!=0){
        SendMessage(OutputWnd, EM_SETSEL, 0, (LPARAM)charlen);
        SendMessage(OutputWnd, EM_REPLACESEL, 0, (LPARAM)"");
      }
    }
    
    strcpy((char *)linebuf, MText);
    strcat((char *)linebuf, CHAR_CRLF);

    DWORD index=(DWORD)SendMessage(OutputWnd, WM_GETTEXTLENGTH, 0, 0);
    SendMessage(OutputWnd, EM_SETSEL, index+1, index+1);
    SendMessage(OutputWnd, EM_REPLACESEL, TRUE, (LPARAM)linebuf);
    SendMessage(OutputWnd, WM_VSCROLL, SB_BOTTOM, 0);
  }
  SendMessage(OutputWnd, WM_SETREDRAW, TRUE, 0);
  InvalidateRect(OutputWnd, NULL, FALSE);

  if(doonce==false){
    doonce=true;
    SendMessage(OutputWnd, WM_LBUTTONDOWN, 0, 0);
    SendMessage(OutputWnd, WM_KILLFOCUS, 0, 0);
  }
}

void logdisp_SetTextColor(COLORREF rgb)
{
  if(OutputWnd!=NULL){

    DWORD index=(DWORD)SendMessage(OutputWnd, WM_GETTEXTLENGTH, 0, 0);
    SendMessage(OutputWnd, EM_SETSEL, index+1, index+1);

    CHARFORMAT2 cf;
    cf.cbSize=sizeof(cf);
    cf.dwMask=CFM_COLOR;
    SendMessage(OutputWnd, EM_GETCHARFORMAT, (WPARAM)SCF_SELECTION, (LPARAM)&cf); //get current format
    cf.dwEffects&=~CFE_AUTOCOLOR;
    cf.crTextColor=rgb;
    SendMessage(OutputWnd, EM_SETCHARFORMAT, (WPARAM)SCF_SELECTION, (LPARAM)&cf); //set new format
  }
}
