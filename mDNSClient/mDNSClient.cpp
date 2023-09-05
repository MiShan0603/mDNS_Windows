// mDNSClient.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "mDNSClient.h"
#include "Utils/Tool.h"
#include "Utils/string_convert.h"
#include "fmt/format.h"
#include "dnssd/dnssd.h"

#define MAX_LOADSTRING 100
#define DETECT_SERVER_TIMER 0x01

HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
HWND g_hWnd = NULL;

#pragma region mDNS 
dnssd_t* g_dnssd = nullptr;
const char regtype[16] = "_myserver._tcp";

#pragma endregion mDNS 

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

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MDNSCLIENT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MDNSCLIENT));

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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MDNSCLIENT));
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
   g_dnssd = dnssd_init_client(&error, regtype);
   if (!g_dnssd)
   {
       return FALSE;
   }


   ::SetTimer(g_hWnd, DETECT_SERVER_TIMER, 2000, 0);

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
        ::KillTimer(g_hWnd, DETECT_SERVER_TIMER);

        if (g_dnssd)
        {
            dnssd_destroy(g_dnssd);
            g_dnssd = NULL;
        }

        PostQuitMessage(0);
        break;
    case WM_TIMER:
    {
        if (wParam == DETECT_SERVER_TIMER)
        {
            dnssd_host_t* hosts = NULL;
            int curRecverNum = dnssd_get_hosts_info(g_dnssd, &hosts);

            for (int i = 0; i < curRecverNum; i++)
            {
                char* utf8Name = ((dnssd_host_t**)hosts)[i]->name;
                char* utf8Addr = ((dnssd_host_t**)hosts)[i]->address[0];
                if (utf8Name == nullptr || utf8Addr == nullptr)
                {
                    continue;
                }

                ((dnssd_host_t**)hosts)[i]->aport; // 我们只用了它
                ((dnssd_host_t**)hosts)[i]->rport;

                std::string name = Utils::to_string(Utils::to_wchar_t(utf8Name));
                std::string address = Utils::to_string(Utils::to_wchar_t(utf8Addr));

                // out put
                OutputDebugStringA(
                    fmt::format(R"( "name":{}, "address":{}, "aport":{}, rport:{} )", 
                        name, address, ((dnssd_host_t**)hosts)[i]->aport, ((dnssd_host_t**)hosts)[i]->rport).c_str()
                );

                for (int index = 0; index < ((dnssd_host_t**)hosts)[i]->recordsCount; index++)
                {
                    std::string key = Utils::to_string(Utils::to_wchar_t(((dnssd_host_t**)hosts)[i]->records[index].key));
                    std::string value = Utils::to_string(Utils::to_wchar_t(((dnssd_host_t**)hosts)[i]->records[index].val));

                    OutputDebugStringA(
                        fmt::format(R"( {}  key:{}, value:{} )", index, key, value).c_str()
                    );
                }
            }

            // free 
            if (hosts)
            {
                for (int index = 0; index < curRecverNum; index++)
                {
                    dnssd_host_t* cp_info = ((dnssd_host_t**)hosts)[index];
                    // dnssd_host_t* cp_info = &m_hosts[index];
                    if (cp_info->name)
                    {
                        free(cp_info->name);
                    }

                    for (int i = 0; i < 4 && cp_info->address[i]; i++)
                    {
                        free(cp_info->address[i]);
                    }

                    if (cp_info->records)
                    {
                        free(cp_info->records);
                    }

                    free(cp_info);
                }

                free(hosts);
                hosts = NULL;
            }
        }
    }
    break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}