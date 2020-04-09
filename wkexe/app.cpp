
#include "app.h"
#include "cmdline.h"
#include "path.h"

#include <wke.h>
#include <stdio.h>
#include <stdlib.h>

#include <shellapi.h>
#pragma comment(lib, "shell32.lib")

#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")


BOOL FixupHtmlFileUrl(LPCWSTR pathOption, LPWSTR urlBuffer, size_t bufferSize)
{
    WCHAR htmlPath[MAX_PATH + 1] = { 0 };

    if (pathOption[0] == 0)
    {
        do
        {
            GetWorkingPath(htmlPath, MAX_PATH, L"index.html");
            if (PathFileExistsW(htmlPath))
                break;

            GetWorkingPath(htmlPath, MAX_PATH, L"main.html");
            if (PathFileExistsW(htmlPath))
                break;

            GetWorkingPath(htmlPath, MAX_PATH, L"wkexe.html");
            if (PathFileExistsW(htmlPath))
                break;    

            GetProgramPath(htmlPath, MAX_PATH, L"index.html");
            if (PathFileExistsW(htmlPath))
                break;    

            GetProgramPath(htmlPath, MAX_PATH, L"main.html");
            if (PathFileExistsW(htmlPath))
                break;

            GetProgramPath(htmlPath, MAX_PATH, L"wkexe.html");
            if (PathFileExistsW(htmlPath))
                break;

            return FALSE;
        }
        while (0);

        swprintf_s(urlBuffer, bufferSize, L"file:///%s", htmlPath);
        return TRUE;
    }

    else//if (!wcsstr(pathOption, L"://"))
    {
        do
        {
            GetWorkingPath(htmlPath, MAX_PATH, pathOption);
            if (PathFileExistsW(htmlPath))
                break;

            GetProgramPath(htmlPath, MAX_PATH, pathOption);
            if (PathFileExistsW(htmlPath))
                break;

            return FALSE;
        }
        while (0);

        swprintf_s(urlBuffer, bufferSize, L"file:///%s", htmlPath);
        return TRUE;
    }

    return FALSE;
}

BOOL FixupHtmlUrl(Application* app)
{
	//LPWSTR htmlOption = L"http://www.fsmeeting.com"; //app->options.htmlFile;
    LPWSTR htmlOption = L"https://www.wasai.life/index.html"; //app->options.htmlFile;
    WCHAR htmlUrl[MAX_PATH + 1] = { 0 };

    // 包含 :// 说明是完整的URL
    if (wcsstr(htmlOption, L"://"))
    {
        wcsncpy_s(app->url, MAX_PATH, htmlOption, MAX_PATH);
        return TRUE;
    }

    // 若不是完整URL，补全之
    if (FixupHtmlFileUrl(htmlOption, htmlUrl, MAX_PATH))
    {
        wcsncpy_s(app->url, MAX_PATH, htmlUrl, MAX_PATH);
        return TRUE;
    }
    // 无法获得完整的URL，出错
    return FALSE;
}

BOOL ProcessOptions(Application* app)
{
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    ParseOptions(argc, argv, &app->options);
    LocalFree(argv);

    return TRUE;
}

// 回调：点击了关闭、返回 true 将销毁窗口，返回 false 什么都不做。
bool HandleWindowClosing(wkeWebView webWindow, void* param)
{
    if (!param)
        return FALSE;

    Application* app = (Application*)param;
    return IDYES == MessageBoxW(NULL, L"确定要退出程序吗？", L"wkexe", MB_YESNO|MB_ICONQUESTION);
}

// 回调：窗口已销毁
void HandleWindowDestroy(wkeWebView webWindow, void* param)
{
    if (!param)
        return;

    Application* app = (Application*)param;
    app->window = NULL;
    PostQuitMessage(0);
}

// 回调：文档加载成功
void HandleDocumentReady(wkeWebView webWindow, void* param)
{
    wkeShowWindow(webWindow, TRUE);

    wkeRunJS(webWindow, "initMXFrame()");
}

// 回调：页面标题改变
void HandleTitleChanged(wkeWebView webWindow, void* param, const wkeString title)
{
    wkeSetWindowTitleW(webWindow, wkeGetStringW(title));
}

// 回调：创建新的页面，比如说调用了 window.open 或者点击了 <a target="_blank" .../>
wkeWebView HandleCreateView(wkeWebView webWindow, void* param, wkeNavigationType navType, const wkeString url, const wkeWindowFeatures* features)
{
    wkeWebView newWindow = wkeCreateWebWindow(WKE_WINDOW_TYPE_POPUP, NULL, features->x, features->y, features->width, features->height);
    wkeShowWindow(newWindow, true);
    return newWindow;
}
bool HandleLoadUrlBegin(wkeWebView webView, void* param, const char *url, void *job)
{
	if (strcmp(url, "http://hook.test/") == 0) {
		wkeNetSetMIMEType(job, "text/html");
        wkeNetChangeRequestUrl(job, url);
		wkeNetSetData(job, "<li>这是个hook页面</li><a herf=\"http://www.baidu.com/\">HookRequest</a>", sizeof("<li>这是个hook页面</li><a herf=\"http://www.baidu.com/\">HookRequest</a>"));
		return true;
	}
	else if (strcmp(url, "http://www.baidu.com/") == 0) {
		wkeNetHookRequest(job);
    }
	return false;
}

void HandleLoadUrlEnd(wkeWebView webView, void* param, const char *url, void *job, void* buf, int len)
{
	wchar_t *str = L"百度一下";
	wchar_t *str1 = L"HOOK一下";

    char* strr = (char*)buf;

	int slen = ::WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL);
	if (slen == 0) return;

	char utf81[10000];
	::WideCharToMultiByte(CP_UTF8, 0, str, -1, &utf81[0], slen, NULL, NULL);

	slen = ::WideCharToMultiByte(CP_UTF8, 0, str1, -1, NULL, 0, NULL, NULL);
	if (slen == 0) return;

	char utf82[10000];
	::WideCharToMultiByte(CP_UTF8, 0, str1, -1, &utf82[0], slen, NULL, NULL);

	const char *b = strstr(static_cast<const char*>(buf), &utf81[0]);
	memcpy((void *)b, &utf82, slen);
	return ;
}

void HandleLoadUrlFail(wkeWebView webView, void* param, const utf8* url, wkeNetJob job)
{
    wchar_t *str = L"百度一下";
    wchar_t *str1 = L"HOOK一下";

    wprintf(str);
}
bool HandleNetResponse(wkeWebView webView, void* param, const utf8* url, wkeNetJob job)
{
    return true;
}

jsValue WKE_CALL_TYPE HandleJsCall(const char *name, jsExecState es, void *param)
{
    printf("name:%s, param:%p.\n", name, param);

    return 0;
}

HWND mainHWND = 0;
HWND CreateMainWindow()
{
    const wchar_t* szClassName = L"wkeWebWindow";
    MSG msg = { 0 };
    WNDCLASSW wndClass = { 0 };
    if (!GetClassInfoW(NULL, szClassName, &wndClass)) {
        wndClass.style = CS_HREDRAW | CS_VREDRAW;
        wndClass.lpfnWndProc = ::DefWindowProc;
        wndClass.cbClsExtra = 200;
        wndClass.cbWndExtra = 200;
        wndClass.hInstance = GetModuleHandleW(NULL);
        wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
        wndClass.hbrBackground = NULL;
        wndClass.lpszMenuName = NULL;
        wndClass.lpszClassName = szClassName;
        RegisterClassW(&wndClass);
    }

    //DWORD styleEx = 0;
    //DWORD styles = 0;

    //if (WKE_WINDOW_STYLE_LAYERED == (styleFlags & WKE_WINDOW_STYLE_LAYERED))
    //    styleEx = WS_EX_LAYERED;

    //if (WKE_WINDOW_STYLE_CHILD == (styleFlags & WKE_WINDOW_STYLE_CHILD))
    //    styles |= WS_CHILD;
    //else
    //    styles |= WS_POPUP|WS_CAPTION|WS_SYSMENU|WS_THICKFRAME;

    //if (WKE_WINDOW_STYLE_BORDER == (styleFlags & WKE_WINDOW_STYLE_BORDER))
    //    styles |= WS_BORDER;

    //if (WKE_WINDOW_STYLE_CAPTION == (styleFlags & WKE_WINDOW_STYLE_CAPTION))
    //    styles |= WS_CAPTION;

    //if (WKE_WINDOW_STYLE_SIZEBOX == (styleFlags & WKE_WINDOW_STYLE_SIZEBOX))
    //    styles |= WS_SIZEBOX;

    //if (WKE_WINDOW_STYLE_SHOWLOADING == (styleFlags & WKE_WINDOW_STYLE_SHOWLOADING))
    //    styles |=  WS_VISIBLE;

    mainHWND = CreateWindowExW(
        0,        // window ex-style
        szClassName,    // window class name
        L"wkeWebWindow", // window caption
        WS_VISIBLE | WS_POPUPWINDOW | WS_CAPTION | WS_MAXIMIZEBOX,         // window style
        1,              // initial x position
        1,              // initial y position
        800,          // initial x size
        600,         // initial y size
        0,         // parent window handle
        NULL,           // window menu handle
        GetModuleHandleW(NULL),           // program instance handle
        0);         // creation parameters
    
    return mainHWND;
}

wkeWebView window2 = nullptr;

BOOL Create2(Application* app)
{
    window2 = wkeCreateWebWindow(WKE_WINDOW_TYPE_CONTROL, mainHWND, 0, 0, 400, 300);
    
    wkeOnWindowClosing(window2, HandleWindowClosing, 0);
    wkeOnWindowDestroy(window2, HandleWindowDestroy, 0);
    wkeOnDocumentReady(window2, HandleDocumentReady, 0);
    wkeOnTitleChanged(window2, HandleTitleChanged, 0);
    wkeOnCreateView(window2, HandleCreateView, 0);
    //wkeOnLoadUrlBegin(app->window, HandleLoadUrlBegin, app);
    //wkeOnLoadUrlEnd(app->window, HandleLoadUrlEnd, app);
    //wkeOnLoadUrlFail(app->window, HandleLoadUrlFail, app);
    //wkeNetOnResponse(app->window, HandleNetResponse, app);

    wkeJsBindFunction(window2, "onOpenApp", HandleJsCall, 0, 1);

    wkeMoveToCenter(window2);
    wkeLoadURLW(window2, app->url);
    wkeShowWindow(window2, TRUE);

    return TRUE;
}

// 创建主页面窗口
BOOL CreateWebWindow(Application* app)
{
    if (app->options.transparent)
        app->window = wkeCreateWebWindow(WKE_WINDOW_TYPE_TRANSPARENT, NULL, 0, 0, 640, 480);
    else
        app->window = wkeCreateWebWindow(WKE_WINDOW_TYPE_CONTROL, mainHWND, 0, 300, 400, 300);

    if (!app->window)
        return FALSE;

    wkeOnWindowClosing(app->window, HandleWindowClosing, app);
    wkeOnWindowDestroy(app->window, HandleWindowDestroy, app);
    wkeOnDocumentReady(app->window, HandleDocumentReady, app);
    wkeOnTitleChanged(app->window, HandleTitleChanged, app);
    wkeOnCreateView(app->window, HandleCreateView, app);
	//wkeOnLoadUrlBegin(app->window, HandleLoadUrlBegin, app);
	//wkeOnLoadUrlEnd(app->window, HandleLoadUrlEnd, app);
    //wkeOnLoadUrlFail(app->window, HandleLoadUrlFail, app);
    //wkeNetOnResponse(app->window, HandleNetResponse, app);

    wkeJsBindFunction(app->window, "onOpenApp", HandleJsCall, app, 1);

    wkeMoveToCenter(app->window);
    wkeLoadURLW(app->window, app->url);
    wkeShowWindow(app->window, TRUE);

    return TRUE;
}

void PrintHelpAndQuit(Application* app)
{
    PrintHelp();
    PostQuitMessage(0);
}

void RunMessageLoop(Application* app)
{
    MSG msg = { 0 };
    while (GetMessageW(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

void RunApplication(Application* app)
{
    memset(app, 0, sizeof(Application));

//     if (!ProcessOptions(app))
//     {
//         PrintHelpAndQuit(app);
//         return;
//     }

    if (!FixupHtmlUrl(app))
    {
        PrintHelpAndQuit(app);
		//打开默认页面
		wcsncpy_s(app->url, MAX_PATH, L"http://www.baidu.com", MAX_PATH);
    }

    CreateMainWindow();

    if (!CreateWebWindow(app))
    {
        PrintHelpAndQuit(app);
        return;
    }
    Create2(app);

    RunMessageLoop(app);
}

void QuitApplication(Application* app)
{
    if (app->window)
    {
        wkeDestroyWebWindow(app->window);
        app->window = NULL;
    }
}
