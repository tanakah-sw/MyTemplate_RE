// richeditole.cxx

#include <windows.h>
#include "richeditole.h"
#include "tips.h"

void InsertHBITMAP(HWND hwnd, HBITMAP hbmp, int w, int h)
{
  LPRICHEDITOLE pRichEditOLE;
  SendMessage(hwnd, EM_GETOLEINTERFACE, 0, (LPARAM)&pRichEditOLE);
  
  IOleClientSite *pOleClientSite;	
  pRichEditOLE->GetClientSite(&pOleClientSite);
  
  LPLOCKBYTES lpLockBytes=NULL;
  CreateILockBytesOnHGlobal(NULL, TRUE, &lpLockBytes);

  IStorage *pStorage;
  StgCreateDocfileOnILockBytes(lpLockBytes,
                               STGM_SHARE_EXCLUSIVE|STGM_CREATE|STGM_READWRITE,
                               0, &pStorage);

  FORMATETC fmt={0};  
  fmt.cfFormat=CF_METAFILEPICT; //  fm.cfFormat = CF_BITMAP;
  fmt.tymed=TYMED_MFPICT;       //  fm.tymed = TYMED_GDI;
  fmt.ptd=NULL;
  fmt.dwAspect=DVASPECT_CONTENT;
  fmt.lindex=-1;

  CImageDataObject *pCDO=new CImageDataObject;
  pCDO->SetImageData(hbmp, w, h, &fmt);

  IOleObject *pOleObject;
  OleCreateStaticFromData(pCDO, IID_IOleObject, OLERENDER_FORMAT, &fmt,  
                          pOleClientSite, pStorage, (void **)&pOleObject);
  CLSID clsid;
  pOleObject->GetUserClassID(&clsid);  


  REOBJECT reobject={0};
  reobject.cbStruct=sizeof(REOBJECT);
  reobject.clsid=clsid;  
  reobject.cp=REO_CP_SELECTION;
  reobject.dvaspect=DVASPECT_CONTENT;
  reobject.polesite=pOleClientSite;
  reobject.poleobj=pOleObject;
  reobject.pstg=pStorage;
  reobject.sizel.cx=(int)((float)w*HIMETRIC_INCH/MONITOR_DPI+0.5);
  reobject.sizel.cy=(int)((float)h*HIMETRIC_INCH/MONITOR_DPI+0.5);
  reobject.dwFlags=REO_DONTNEEDPALETTE|REO_BELOWBASELINE;

  SendMessage(hwnd, WM_SETREDRAW, FALSE, 0);
  {
    DWORD index=(DWORD)SendMessage(hwnd, WM_GETTEXTLENGTH, 0, 0);
    SendMessage(hwnd, EM_SETSEL, index+1, (LPARAM)index+1);
    pRichEditOLE->InsertObject(&reobject);
    SendMessage(hwnd, EM_REPLACESEL, TRUE, (WPARAM)CHAR_CRLF); 
    SendMessage(hwnd, WM_VSCROLL, SB_BOTTOM, 0);
  }
  SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);
  InvalidateRect(hwnd, NULL, FALSE);
  
  pOleObject->Release();

  pCDO->FreeImageData();
  delete pCDO;

  pStorage->Release();
  pOleClientSite->Release();
  pRichEditOLE->Release();
}

bool extractHBITMAP(HWND hwnd, HBITMAP *hbmp, int *w, int *h)
{
  LPRICHEDITOLE pREOLE=NULL;
  SendMessage(hwnd, EM_GETOLEINTERFACE, 0, (LPARAM)&pREOLE);

  HRESULT hr=S_FALSE;
  
  if(pREOLE!=NULL){
    REOBJECT reobject={0};
    reobject.cbStruct=sizeof(REOBJECT);
    hr=pREOLE->GetObject(-1, &reobject, REO_GETOBJ_POLEOBJ);
    if(hr==S_OK){
      SIZEL sizel;
      reobject.poleobj->GetExtent(DVASPECT_CONTENT, &sizel);
      
      *w=(int)((float)reobject.sizel.cx/(HIMETRIC_INCH/MONITOR_DPI)+0.5);
      *h=(int)((float)reobject.sizel.cy/(HIMETRIC_INCH/MONITOR_DPI)+0.5);
      
      HDC hdc=GetDC(NULL);
      {
        HBITMAP holdbmp;
        
        HDC hmemdc=CreateCompatibleDC(hdc);
        *hbmp=CreateCompatibleBitmap(hdc, *w, *h);

        holdbmp=(HBITMAP)SelectObject(hmemdc, *hbmp);
        
        RECT rc;
        SetRect(&rc, 0, 0, *w, *h);
        hr=OleDraw(reobject.poleobj, DVASPECT_CONTENT, hmemdc, &rc);

        SelectObject(hmemdc, holdbmp);
        DeleteDC(hmemdc);
      }
      ReleaseDC(NULL, hdc);
    }
    pREOLE->Release();
  }
  if(hr==S_OK) return true;
  return false;
}

void CImageDataObject::FreeImageData()
{
  LPMETAFILEPICT lpmf=(LPMETAFILEPICT)m_stgm.hGlobal;
  if(lpmf!=NULL){
    DeleteMetaFile(lpmf->hMF);
    lpmf=NULL;
  }
}

void CImageDataObject::SetImageData(HBITMAP hbmp, int w, int h, FORMATETC *fmt)
{
  HDC hmfdc=(HDC)CreateMetaFile(NULL);
  {
    SetMapMode(hmfdc, MM_ANISOTROPIC);
    SetWindowOrgEx(hmfdc, 0, 0, NULL);
    SetWindowExtEx(hmfdc, w, h, NULL);
    SetStretchBltMode(hmfdc, HALFTONE);
    
    HDC hdc=GetDC(NULL);
    HDC hmemdc=CreateCompatibleDC(GetDC(NULL));
    SelectObject(hmemdc, hbmp);
    BitBlt(hmfdc, 0, 0, w+1, h+1, hmemdc, 0, 0, SRCCOPY);
    DeleteDC(hmemdc);
    ReleaseDC(NULL, hdc);
  }
  HMETAFILE hmf=CloseMetaFile(hmfdc);

  LPMETAFILEPICT lpmf=(LPMETAFILEPICT)GlobalAlloc(GMEM_FIXED, sizeof(METAFILEPICT));
  lpmf->hMF=hmf;
  lpmf->mm=MM_ANISOTROPIC;
  lpmf->xExt=lpmf->yExt=0;

  m_stgm.pUnkForRelease=NULL;
  m_stgm.tymed=TYMED_MFPICT;
  m_stgm.hGlobal=lpmf;

  this->SetData(fmt, &m_stgm, FALSE);
}
