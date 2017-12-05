// logging.h

extern void logdisp_SetCtlColor(COLORREF cr);
extern void logdisp_SetTextColor(COLORREF cr);
extern void logdisp_PrintTextf(char *fmt, ...);
extern void logdisp_DrawBitmap(HBITMAP hbmp, int w, int h);

extern void logdisp_SetOutputWnd(HWND hwnd);
extern LRESULT CALLBACK logdisp_dlgProcRE(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam);

