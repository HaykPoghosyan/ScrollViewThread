// ScrollView.cpp : Defines the entry 7point for the application.
//

#include "framework.h"
#include "ScrollView.h"

#define MAX_LOADSTRING 100

#define WM_USER 0x0400

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
vector <unsigned char> file_data;
HDC hCacheDC = NULL;
bool IsLoad = false;
int nScrollOffset = 0;

HANDLE hThread = NULL;

#define COLOR_TO_RGB(Color)  RGB((Color >> 5) * 255 / 7, ((Color >> 2) & 0x07) * 255 / 7, (Color & 0x03) * 255 / 3)

SIZE CalcBitmapSize(const RECT& Rect)
{
    SIZE BitmapSize;

    BitmapSize.cx = Rect.right - Rect.left;
    BitmapSize.cy = file_data.size() / BitmapSize.cx;

    if (file_data.size() % BitmapSize.cx)
        BitmapSize.cy++;

    int WindowHeight = Rect.bottom - Rect.top;

    if (BitmapSize.cy < WindowHeight)
        BitmapSize.cy = WindowHeight;

    return BitmapSize;
}

struct MyData {
    HWND hWnd;
    SIZE size;
};

DWORD WINAPI MyThread(LPVOID lpParameter)
{
    IsLoad = true;
    MyData m = *(MyData*)lpParameter;

    RECT Rect;
    GetClientRect(m.hWnd, &Rect);

    // Get screen DC
    HDC hScreenDC = GetDC(NULL);

    // Create a compatible DC (memory DC) of the current context
    HDC hDrawDC = ::CreateCompatibleDC(hScreenDC);

    // Create a compatible Bitmap
    HBITMAP hDrawBitmap = CreateCompatibleBitmap(hScreenDC, m.size.cx, m.size.cy);

    ::ReleaseDC(NULL, hScreenDC);

    //Load bitmap to memory DC
    HBITMAP hOldBitmap = (HBITMAP)::SelectObject(hDrawDC, hDrawBitmap);

    PatBlt(hDrawDC, 0, 0, m.size.cx, m.size.cy, WHITENESS);

    for (int i = 0; i < file_data.size(); i++)
    {
        int y = i / m.size.cx;
        int x = i % m.size.cx;
        unsigned char byte = file_data[i];
        COLORREF color = COLOR_TO_RGB(byte);
        SetPixel(hDrawDC, x, y, color);
    }

    ::DeleteObject(hOldBitmap);

    if (hDrawDC)
    {
        HGDIOBJ hCacheBitmap = GetCurrentObject(hDrawDC, OBJ_BITMAP);
        DeleteDC(hCacheDC);
        DeleteObject(hCacheDC);
    }

    hCacheDC = hDrawDC;

    InvalidateRect(m.hWnd, &Rect, FALSE);

    PostMessage(m.hWnd, WM_USER, 0, 0);

    return 0;
}

void UpdateCache(HWND hWnd, const SIZE& BitmapSize)
{
    TerminateThread(hThread, NULL);
    MyData m;
    m.hWnd = hWnd;
    m.size = BitmapSize;
    hThread = CreateThread(NULL, 0, MyThread, &m, 0, NULL);   
}


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
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SCROLLVIEW, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SCROLLVIEW));

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

    // Wait for thread to finish execution
    //WaitForSingleObject(hThread, INFINITE);

    //// Thread handle must be closed when no longer needed
    //CloseHandle(hThread);
    return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SCROLLVIEW));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_SCROLLVIEW);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

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
//  WM_PAINT    - paint the main window
//  WM_SIZE     - change scroll ranges and refresh drawing cache
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_USER:
    {
        WaitForSingleObject(hThread, INFINITE);

        // Thread handle must be closed when no longer needed
        CloseHandle(hThread);
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        RECT Rect;
        GetClientRect(hWnd, &Rect);

        ::BitBlt(hdc, 0, 0, Rect.right - Rect.left, Rect.bottom - Rect.top, hCacheDC, 0, nScrollOffset, SRCCOPY);

        EndPaint(hWnd, &ps);
    }
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_OPEN:
            {
                OPENFILENAME ofn;                // common dialog box structure
                TCHAR szFile[260] = { 0 };       // if using TCHAR macros

                // Initialize OPENFILENAME
                ZeroMemory(&ofn, sizeof(ofn));
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = hWnd;
                ofn.lpstrFile = szFile;
                ofn.nMaxFile = sizeof(szFile);
                ofn.lpstrFilter = _T("All\0*.*\0");
                ofn.nFilterIndex = 1;
                ofn.lpstrFileTitle = NULL;
                ofn.nMaxFileTitle = 0;
                ofn.lpstrInitialDir = NULL;
                ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

                if (GetOpenFileName(&ofn) == TRUE)
                {
                    HANDLE hFile = CreateFile(ofn.lpstrFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);

                    file_data.clear();

                    unsigned char buffer[1000];

                    DWORD bytesRead = 0;
                    do
                    {
                        if (!ReadFile(hFile, buffer, 1000, &bytesRead, NULL))
                            break;

                        for (int i = 0; i < (int)bytesRead; i++)
                            file_data.push_back(buffer[i]);
                    }
                    while (bytesRead != 0);

                    CloseHandle(hFile);

                    // First drawing after loading
                    RECT Rect;
                    GetClientRect(hWnd, &Rect);
                    SIZE BitmapSize = CalcBitmapSize(Rect);
                    UpdateCache(hWnd, BitmapSize);
                    InvalidateRect(hWnd, &Rect, FALSE);
                }
                break;
            }
            case IDM_ABOUT:
            {
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            }
            case IDM_EXIT:
            {
                DestroyWindow(hWnd);
                break;
            }
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_VSCROLL:
    {
        SCROLLINFO si;
        si.cbSize = sizeof(SCROLLINFO);
        si.fMask = SIF_ALL;
        GetScrollInfo(hWnd, SB_VERT, &si);

        switch (LOWORD(wParam))
        {
            // User clicked the left arrow.
        case SB_LINELEFT:
            si.nPos -= 1;
            break;

            // User clicked the right arrow.
        case SB_LINERIGHT:
            si.nPos += 1;
            break;

            // User clicked the scroll bar shaft left of the scroll box.
        case SB_PAGELEFT:
            si.nPos -= si.nPage;
            break;

            // User clicked the scroll bar shaft right of the scroll box.
        case SB_PAGERIGHT:
            si.nPos += si.nPage;
            break;

            // User dragged the scroll box.
        case SB_THUMBTRACK:
            si.nPos = si.nTrackPos;
            break;

        default:
            break;
        }

        if (si.nPos < 0)
            si.nPos = si.nMin;

        if (si.nPos > si.nMax - si.nPage)
            si.nPos = si.nMax - si.nPage;

        si.fMask = SIF_POS;
        SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
        nScrollOffset = si.nPos;

        RECT Rect;
        GetClientRect(hWnd, &Rect);
        InvalidateRect(hWnd, &Rect, TRUE);
        UpdateWindow(hWnd);
        break;
    }
    case WM_SIZE:
    {
        RECT Rect;
        GetClientRect(hWnd, &Rect);
        InvalidateRect(hWnd, &Rect, FALSE);

        SIZE BitmapSize = CalcBitmapSize(Rect);
        int WindowHeight = Rect.bottom - Rect.top;

        // Refresh scroll bars first

        DWORD style = GetWindowLong(hWnd, GWL_STYLE);

        bool bResize = FALSE;

        if (BitmapSize.cy > WindowHeight)
        {
            if (!(style & WS_VSCROLL))
            {
                SetWindowLong(hWnd, GWL_STYLE, style | WS_VSCROLL);
                bResize = TRUE;
            }

            SCROLLINFO si;
            si.cbSize = sizeof(SCROLLINFO);
            si.nMin = 0;
            si.nMax = BitmapSize.cy;
            si.nPage = WindowHeight;
            si.nPos = nScrollOffset;
            si.nTrackPos = 0;
            si.fMask = SIF_ALL;

            if (si.nPos > si.nMax - si.nPage)
                si.nPos = si.nMax - si.nPage;

            nScrollOffset = si.nPos;

            SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
        }
        else
        {
            if (style & WS_VSCROLL)
            {
                SetWindowLong(hWnd, GWL_STYLE, style & ~WS_VSCROLL);
                nScrollOffset = 0;
                bResize = TRUE;
            }
        }

        if (bResize)
        {
            // Scroll bars appear or disappear, so we need to resize windows and update it again - WM_SIZE will be called again from the SetWindowPos
            SetWindowPos(hWnd, 0, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOREPOSITION | SWP_NOSIZE | SWP_NOMOVE);
            
            // Break WM_SIZE handling to avoid unnecessary redrawing / cache update
            break;
        }

        UpdateCache(hWnd, BitmapSize);
        break;
    }
    case WM_PAINT:
        {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);


        RECT Rect;
        GetClientRect(hWnd, &Rect);

        if (IsLoad) 
        {
            TextOut(hdc, (Rect.right - Rect.left) / 2 - 40, (Rect.bottom - Rect.top) / 2 - 10, L"IN PROGRESS!", 12);
        }
        else 
        {
            ::BitBlt(hdc, 0, 0, Rect.right - Rect.left, Rect.bottom - Rect.top, hCacheDC, 0, nScrollOffset, SRCCOPY);
        }
        EndPaint(hWnd, &ps);
        }
        break;
    case WM_ERASEBKGND:
        return 1;
        break;
    case WM_DESTROY:
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
