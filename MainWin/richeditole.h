// richeditole.h
#include <richedit.h>
#include <richole.h>
#include "resource.h"

#define IDM_RE_OPEN                                40101
#define IDM_RE_SAVE                                40102

extern void InsertHBITMAP(HWND hwnd, HBITMAP hbmp, int w, int h);
extern bool extractHBITMAP(HWND hwnd, HBITMAP *hbmp, int *w, int *h);

class IExRichEditOleCallback : public IRichEditOleCallback
{
public:
  void SetHWND(HWND hwnd)
  {
    hwndRE=hwnd;
  }
private:
  DWORD m_dwRef;
  HWND hwndRE;
public:
  IExRichEditOleCallback(): m_dwRef(0), hwndRE(NULL)
  {
  }
  ~IExRichEditOleCallback()
  {
  }
  ULONG STDMETHODCALLTYPE AddRef()
  {
    return ++m_dwRef;
  }
  ULONG STDMETHODCALLTYPE Release()
  {
    if(--m_dwRef==0){
      delete this;
      return 0;
    }
    return m_dwRef;
  }
  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void ** ppvObject)
  {
    HRESULT hr=S_OK;
    *ppvObject=NULL;
    if((iid==IID_IUnknown)||(iid==IID_IDataObject)){
      *ppvObject=this;
      AddRef();
      hr=S_OK;
    }else{
      hr=E_NOINTERFACE;
    }
    return hr;
  }
  HRESULT STDMETHODCALLTYPE GetContextMenu(WORD seltyp, LPOLEOBJECT, CHARRANGE *, HMENU *lphmenu)
  {
    if(seltyp==SEL_OBJECT){
      HMENU hmenu=CreatePopupMenu();
      POINT pt;
      GetCursorPos(&pt);
      AppendMenu(hmenu, MFT_STRING, IDM_RE_OPEN, "Open");
      AppendMenu(hmenu, MFT_STRING, IDM_RE_SAVE, "Save as bitmap");
      if(hwndRE!=NULL) TrackPopupMenu(hmenu, 0, pt.x, pt.y, 0, hwndRE, NULL);
      lphmenu=&hmenu;
    }
    return S_OK;
  }
  HRESULT STDMETHODCALLTYPE ContextSensitiveHelp(BOOL)
  {
    return S_OK;
  }
  HRESULT STDMETHODCALLTYPE DeleteObject(LPOLEOBJECT)
  {
    return S_OK;
  }
  HRESULT STDMETHODCALLTYPE GetClipboardData(CHARRANGE *, DWORD, LPDATAOBJECT *)
  {
    return E_NOTIMPL;
  }
  HRESULT STDMETHODCALLTYPE GetDragDropEffect(BOOL, DWORD, LPDWORD)
  {
    return S_OK;
  }
  HRESULT STDMETHODCALLTYPE GetInPlaceContext(LPOLEINPLACEFRAME *, LPOLEINPLACEUIWINDOW *, LPOLEINPLACEFRAMEINFO)
  {
    return S_OK;
  }
  HRESULT STDMETHODCALLTYPE GetNewStorage(LPSTORAGE *)
  {
    return S_OK;
  }
  HRESULT STDMETHODCALLTYPE QueryAcceptData(LPDATAOBJECT, CLIPFORMAT *, DWORD, BOOL, HGLOBAL)
  {
    return S_OK;
  }
  HRESULT STDMETHODCALLTYPE QueryInsertObject(LPCLSID, LPSTORAGE, LONG)
  {
    return S_OK;
  }
  HRESULT STDMETHODCALLTYPE ShowContainerUI(BOOL)
  {
    return S_OK;
  }
};

class CImageDataObject : public IDataObject
{
public:
  void SetImageData(HBITMAP hBitmap, int w, int h, FORMATETC *fmt);
  void FreeImageData();

private:
  ULONG	m_dwRef;
  BOOL	m_bRel;
  STGMEDIUM m_stgm;

public:
  CImageDataObject(): m_dwRef(0), m_bRel(FALSE)
  {
    LPDATAOBJECT lpDataObject;
    QueryInterface(IID_IDataObject, reinterpret_cast<void **>(&lpDataObject));
  }
  ~CImageDataObject()
  {
    if(m_bRel) ReleaseStgMedium(&m_stgm);
  }
  ULONG STDMETHODCALLTYPE AddRef()
  {
    return ++m_dwRef;
  }
  ULONG STDMETHODCALLTYPE Release()
  {
    if(--m_dwRef==0){
      delete this;
      return 0;
    }
    return m_dwRef;
  }
  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void **ppvObject)
  {
    HRESULT hr=S_OK;
    *ppvObject=NULL;
    if((iid==IID_IUnknown)||(iid==IID_IDataObject)){
      *ppvObject=this;
      AddRef();
      hr=S_OK;
    }else{
      hr=E_NOINTERFACE;
    }
    return hr;
  }

  HRESULT STDMETHODCALLTYPE SetData(FORMATETC *, STGMEDIUM *pmedium , BOOL fRelease)
  {
    m_stgm=*pmedium;
    m_bRel=fRelease;
    return S_OK;
  }
  HRESULT STDMETHODCALLTYPE GetData(FORMATETC *pfmtetc, STGMEDIUM *pmedium)
  {
    if(pfmtetc->cfFormat==CF_METAFILEPICT){
      pmedium->pUnkForRelease=NULL;
      pmedium->tymed=TYMED_MFPICT;      //      pmedium->tymed = TYMED_GDI;
      pmedium->hGlobal=m_stgm.hGlobal;  //      pmedium->hBitmap = m_stgmed.hBitmap;
      return S_OK;
    }
    return E_NOTIMPL;
  }
  HRESULT STDMETHODCALLTYPE GetDataHere(FORMATETC *,  STGMEDIUM *)
  {
    return E_NOTIMPL;
  }
  HRESULT STDMETHODCALLTYPE QueryGetData(FORMATETC *)
  {
    return E_NOTIMPL;
  }
  HRESULT STDMETHODCALLTYPE GetCanonicalFormatEtc(FORMATETC *, FORMATETC *)
  {
    return E_NOTIMPL;
  }
  HRESULT STDMETHODCALLTYPE EnumFormatEtc(DWORD, IEnumFORMATETC **)
  {
    return E_NOTIMPL;
  }
  HRESULT STDMETHODCALLTYPE DAdvise(FORMATETC *, DWORD, IAdviseSink *, DWORD *)
  {
    return E_NOTIMPL;
  }
  HRESULT STDMETHODCALLTYPE DUnadvise(DWORD)
  {
    return E_NOTIMPL;
  }
  HRESULT STDMETHODCALLTYPE EnumDAdvise(IEnumSTATDATA **)
  {
    return E_NOTIMPL;
  }
};
