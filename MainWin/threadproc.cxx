// threadproc.cxx
#include "mainwin.h"

// richedit-logger
DWORD WINAPI mainWin::thrProcRE(PVOID *lpparam)
{
  mainWin *_this=reinterpret_cast<mainWin *>(lpparam);

  OleInitialize(NULL);
  _this->hdlgRE=CreateDialogParam(hAppInstance, MAKEINTRESOURCE(IDD_DIALOG_RE), _this->hdlg,
                                  (DLGPROC)logdisp_dlgProcRE, (LONG_PTR)IDC_RICHEDIT_LOG);
  MSG msg;
  SetEvent(_this->hevent_initializeRE);

  DWORD cnt=0;
  DWORD prv=timeGetTime();
  DWORD nxt=timeGetTime()+_this->LoopDurationRE;
  for(;;){
    if(PeekMessage(&msg,0, 0, 0, PM_REMOVE)){
      if(msg.message==WM_COMMAND){
        if(msg.wParam==ID_INSERT_HBMP_AND_DELETE){
          HBITMAP hbmp=(HBITMAP)msg.lParam;
          if(hbmp!=NULL){
            BITMAP bm;
            GetObject(hbmp, sizeof(BITMAP), &bm);
            
            int w, h, b;
            w=bm.bmWidth;
            h=bm.bmHeight;
            b=bm.bmBitsPixel;
            
            logdisp_PrintTextf("insert image: size=%dx%d, bit=%d", w, h, b);
            logdisp_DrawBitmap(hbmp, w, h);
            DeleteObject(hbmp);
          }
        }
      }
      DispatchMessage(&msg);
    }
    DWORD now=timeGetTime();
    if(now>=nxt){
      prv=now;
      nxt=now+_this->LoopDurationRE;
    }
    int diff=nxt-timeGetTime();
    DWORD res;
    if(diff>0){
      res=MsgWaitForMultipleObjects(1, &_this->hevent_finalizeRE, FALSE, diff, QS_ALLEVENTS);
    }else{
      res=WaitForMultipleObjects(1, &_this->hevent_finalizeRE, FALSE, 0);
    }
    if(res==WAIT_OBJECT_0+0) break;
    cnt++;
  }
  OleUninitialize();
  return 0;
}
