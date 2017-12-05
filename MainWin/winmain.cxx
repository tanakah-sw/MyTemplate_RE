// winmain.cxx
#include "mainwin.h"

// global-variables //////////////////////////////////////////////////////////////////////
HINSTANCE              hAppInstance;
char                   HomeFolderName[MAX_PATH];
CRITICAL_SECTION       c_section;
ID2D1Factory          *pD2D_Factory;
IWICImagingFactory    *pWIC_Factory;
IDWriteFactory        *pDWR_Factory;

//////////////////////////////////////////////////////////////////////////////////////////
LONG WINAPI ExceptionFilter(EXCEPTION_POINTERS *ExceptionInfo)
{
  SystemParametersInfo(SPI_SETCURSORS, 0, NULL, 0);

  char mes[256];
  sprintf(mes, "Unhandled error occurred.(address:0x%08x)",
          PtrToUlong(ExceptionInfo->ExceptionRecord->ExceptionAddress));
  MessageBox(NULL, mes, "ExceptionError", MB_OK|MB_TOPMOST|MB_APPLMODAL);

  //  return EXCEPTION_CONTINUE_SEARCH;
  //  return EXCEPTION_CONTINUE_EXECUTION;
  return EXCEPTION_EXECUTE_HANDLER;
}

//////////////////////////////////////////////////////////////////////////////////////////
#ifdef USE_WPF
[System::STAThreadAttribute]
#endif
int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
  // prevent multiple start
  HANDLE hmutex;
  {
    SECURITY_DESCRIPTOR sd;
    InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(&sd, TRUE, 0, FALSE);

    SECURITY_ATTRIBUTES secAttribute;
    secAttribute.nLength=sizeof(SECURITY_ATTRIBUTES);
    secAttribute.lpSecurityDescriptor=&sd;
    secAttribute.bInheritHandle=TRUE;
    hmutex=CreateMutex(&secAttribute, FALSE, MAINWIN_CLASS_NAME);

    if((hmutex==NULL)||(GetLastError()==ERROR_ALREADY_EXISTS)){
      HWND hwnd_fs=FindWindow(MAINWIN_CLASS_NAME, NULL);
      if(hwnd_fs!=NULL){
        ShowWindow(hwnd_fs, SW_SHOWNORMAL);
        SetForegroundWindow(hwnd_fs);
        SetActiveWindow(hwnd_fs);
      }else{
        hwnd_fs=FindWindow(NULL, MAINWIN_WINDOW_NAME);
        if(hwnd_fs!=NULL){
          SetForegroundWindow(hwnd_fs);
          SetActiveWindow(hwnd_fs);
          SetWindowPos(hwnd_fs, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
        }
      }
      return 0;
    }
  }

  // global variables
  hAppInstance=hInstance;
  GetModuleFileName(NULL, HomeFolderName, MAX_PATH); PathRemoveFileSpec(HomeFolderName);
  InitializeCriticalSection(&c_section);

  // Direct2D/DWrite/WIC
  CoInitialize(NULL);
  D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2D_Factory);
  DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown **>(&pDWR_Factory));
  CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, __uuidof(IWICImagingFactory), reinterpret_cast<LPVOID *>(&pWIC_Factory));

  // exception handler
  HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);
  SetUnhandledExceptionFilter(ExceptionFilter);

  // common library
  INITCOMMONCONTROLSEX icex;
  icex.dwSize=sizeof(INITCOMMONCONTROLSEX); icex.dwICC=ICC_COOL_CLASSES|ICC_WIN95_CLASSES;
  InitCommonControlsEx(&icex);
  HMODULE hre_dll=LoadLibrary("RichEd32.dll");

#ifdef USE_EXTERNALDLL
  SetDllDirectory(dllfoldername);
#endif

  // initialize
  timeBeginPeriod(1);
  
  mainWin *mainwin=new mainWin();
  {
    mainwin->Initialize();
    mainwin->Show();
    mainwin->RunMessageLoop();
    mainwin->Uninitialize();
  }
  delete mainwin;

  // finalize
  timeEndPeriod(1);
  SystemParametersInfo(SPI_SETCURSORS, 0, NULL, 0);

  if(hre_dll!=NULL) FreeLibrary(hre_dll);

  SafeRelease(&pWIC_Factory);
  SafeRelease(&pDWR_Factory);
  SafeRelease(&pD2D_Factory);
  CoUninitialize();
  
  DeleteCriticalSection(&c_section);
  CloseHandle(hmutex);

  return 0;
}

