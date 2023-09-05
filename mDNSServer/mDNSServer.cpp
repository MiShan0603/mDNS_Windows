// mDNSServer.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "mDNSServer.h"
#include "Utils/Tool.h"
#include "Utils/string_convert.h"

#include "dnssd/dnssd.h"

#define MAX_LOADSTRING 100
#define SERVER_TIMER 0x01

HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
HWND g_hWnd = NULL;

#pragma region mDNS 
dnssd_t* g_dnssd_srv = nullptr;
const char name[64] = "测试服务";
std::string utf8Name;
const char regtype[16] = "_myserver._tcp";
const int port = 10001;

dnssd_record_kv_t* records = NULL;
int recordsCount = 0;

#pragma endregion mDNS 

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MDNSSERVER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MDNSSERVER));

    MSG msg;

    // 主消息循环:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MDNSSERVER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
       0, 0, 640, 360, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   Utils::Tool::Instance()->CenterWindow(hWnd);
   g_hWnd = hWnd;

   int error = 0;
   g_dnssd_srv = dnssd_init_srv(&error);
   if (!g_dnssd_srv)
   {
       return FALSE;
   }

   // for test ...
   recordsCount = 5;
   records = new dnssd_record_kv_t[recordsCount];
   for (int i = 0; i < recordsCount - 1; i++)
   {
       memset(records[i].key, 0, 16);
       memset(records[i].val, 0, 128);

       sprintf(records[i].key, "key_%d", i);
       sprintf(records[i].val, "value_%d", i);
   }

   {
       memset(records[recordsCount - 1].key, 0, 16);
       memset(records[recordsCount - 1].val, 0, 128);

       std::wstring val = L"测试value";
       std::string utf8Val = Utils::to_utf8(val);

       sprintf(records[recordsCount - 1].key, "key_4");
       sprintf(records[recordsCount - 1].val, "%s", utf8Val.c_str());
   }

   utf8Name = Utils::to_utf8(Utils::to_wstring(name));
   dnssd_register_server(g_dnssd_srv, utf8Name.c_str(), port, regtype, records, recordsCount);

   // 有些网络情况下，可能无法搜索到，我们这里反复注册
   // In some network situations, it may not be possible to search, we repeatedly register here
   ::SetTimer(g_hWnd, SERVER_TIMER, 3000, 0);

   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 分析菜单选择:
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
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        ::KillTimer(g_hWnd, SERVER_TIMER);

        dnssd_unregister_server(g_dnssd_srv);
        dnssd_destroy(g_dnssd_srv);

        if (records)
        {
            delete[]records;
        }

        PostQuitMessage(0);
        break;
    case WM_TIMER:
    {
        if (wParam == SERVER_TIMER)
        {
            dnssd_unregister_server(g_dnssd_srv);

            Sleep(300);

            dnssd_register_server(g_dnssd_srv, utf8Name.c_str(), port, regtype, records, recordsCount);
        }
    }
    break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}