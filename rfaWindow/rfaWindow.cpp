#include "framework.h"
#include "rfaWindow.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HBITMAP hBitmap = nullptr;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    const wchar_t szUniqueNamedMutex[] = L"rfaWindow";
    HANDLE hHandle = CreateMutex(NULL, TRUE, szUniqueNamedMutex);
    if (ERROR_ALREADY_EXISTS == GetLastError())
    {
        // Program already running somewhere
        return(1); // Exit program
    }

    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_RFAWINDOW, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_RFAWINDOW));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    ReleaseMutex(hHandle);
    CloseHandle(hHandle);
    
    return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{


    WNDCLASSEX wcex = { 0 };
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.lpfnWndProc = WndProc;        // function which will handle messages
    wcex.hInstance = hInstance;
    wcex.lpszClassName = szWindowClass;

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(
       szWindowClass,
       szTitle,       
       WS_POPUP,
       CW_USEDEFAULT, CW_USEDEFAULT, // Initial x, y position
       430, 460,                     // Width, height
       nullptr,
       nullptr,
       hInstance,
       nullptr
   );

   CreateWindow(
       TEXT("STATIC"),              // Predefined static control class
       TEXT("Вы не можете редактировать данное семейство. Пожалуйста, закройте его!"),
       WS_CHILD | WS_VISIBLE,       // Child window, visible, centered text
       15, 15,                      // X, Y position
       350, 35,                     // Width, Height
       hWnd,
       NULL,
       hInstance,
       NULL                          // No creation data
   );

   HWND hwndButton = CreateWindowEx(WS_EX_TOOLWINDOW,
       L"BUTTON", // Predefined class for a button
       L"Хорошо, сейчас",  // Text displayed on the button
       WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, // Styles
       220,             // X coordinate
       405,             // Y coordinate
       200,             // Button width
       35,              // Button height
       hWnd,            // Handle to the parent window
       (HMENU)IDM_EXIT, // Unique identifier for the button
       hInstance,       // Instance handle
       NULL             // No creation parameters
   );

   /* HWND_TOPMOST block */
   RECT rc;
   GetWindowRect(hWnd, &rc);

   int xPos = (GetSystemMetrics(SM_CXSCREEN) - rc.right) / 2;
   int yPos = (GetSystemMetrics(SM_CYSCREEN) - rc.bottom) / 2;
   SetWindowPos(hWnd, HWND_TOPMOST, xPos - 200, yPos - 100, 0, 0, SWP_NOSIZE);

   long style = GetWindowLong(hWnd, GWL_EXSTYLE);
   style &= ~(WS_VISIBLE);    // this works - window become invisible 

   style |= WS_EX_TOOLWINDOW;   // flags don't work - windows remains in taskbar
   style &= ~(WS_EX_APPWINDOW);

   ShowWindow(hWnd, SW_HIDE); // hide the window
   SetWindowLong(hWnd, GWL_EXSTYLE, style); // set the style
   ShowWindow(hWnd, SW_SHOW);

   if (!hWnd)
   {
       return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        hBitmap = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP_RFA), IMAGE_BITMAP, 0, 0, 0);
        break;
    case WM_CTLCOLORSTATIC:
    {
        HDC hdcStatic = (HDC)wParam;
        SetBkMode(hdcStatic, TRANSPARENT);
        // Optionally, set the text color
        // SetTextColor(hdcStatic, RGB(255, 0, 0)); // Example: Red text
        return (LRESULT)GetStockObject(NULL_BRUSH); // Return a NULL_BRUSH to make the background transparent
    }
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        PAINTSTRUCT     ps;
        HDC             hdc;
        BITMAP          bitmap;
        HDC             hdcMem;
        HGDIOBJ         oldBitmap;

        hdc = BeginPaint(hWnd, &ps);

        hdcMem = CreateCompatibleDC(hdc);
        oldBitmap = SelectObject(hdcMem, hBitmap);

        GetObject(hBitmap, sizeof(bitmap), &bitmap);
        BitBlt(hdc, 0, 0, bitmap.bmWidth, bitmap.bmHeight, hdcMem, 0, 0, SRCCOPY);

        SelectObject(hdcMem, oldBitmap);
        DeleteDC(hdcMem);

        EndPaint(hWnd, &ps);
        break;
        break;
    case WM_DESTROY:
        if (hBitmap != NULL) {
            DeleteObject(hBitmap);
        }
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
