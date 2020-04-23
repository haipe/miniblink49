
#include <windows.h>
#include <stdlib.h>
#include <tchar.h>
#include <DbgHelp.h>
#include "app.h"


#pragma comment(lib, "dbghelp.lib")

#ifdef UNICODE
#define TSprintf	wsprintf
#else
#define TSprintf	sprintf
#endif

// 启动自动生成dump文件的话，只需要在main函数开始处
// 调用该函数（EnableAutoDump）即可
void EnableAutoDump();
// 其它函数
LONG ApplicationCrashHandler(EXCEPTION_POINTERS *pException);
void CreateDumpFile(LPCWSTR lpstrDumpFilePathName, EXCEPTION_POINTERS *pException);

LONG ApplicationCrashHandler(EXCEPTION_POINTERS *pException)
{
    TCHAR szDumpDir[MAX_PATH] = { 0 };
    TCHAR szDumpFile[MAX_PATH] = { 0 };
    TCHAR szMsg[MAX_PATH] = { 0 };
    SYSTEMTIME	stTime = { 0 };

    // 构建dump文件路径
    GetLocalTime(&stTime);
    ::GetCurrentDirectory(MAX_PATH, szDumpDir);
    TSprintf(szDumpFile, _T("%s\\%04d%02d%02d_%02d%02d%02d.dmp"), szDumpDir,
        stTime.wYear, stTime.wMonth, stTime.wDay,
        stTime.wHour, stTime.wMinute, stTime.wSecond);
    // 创建dump文件
    CreateDumpFile(szDumpFile, pException);

    // 这里弹出一个错误对话框并退出程序
    TSprintf(szMsg, _T("I'm so sorry, but the program crashed.\r\ndump file : %s"), szDumpFile);
    FatalAppExit(-1, szMsg);

    return EXCEPTION_EXECUTE_HANDLER;
}

void EnableAutoDump()
{
    SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)ApplicationCrashHandler);
}

inline void CreateDumpFile(LPCWSTR lpstrDumpFilePathName, EXCEPTION_POINTERS *pException)
{
    HANDLE hDumpFile = CreateFile(lpstrDumpFilePathName, GENERIC_READ | GENERIC_WRITE,
        0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if ((hDumpFile != NULL) && (hDumpFile != INVALID_HANDLE_VALUE))
    {
        // Dump信息
        MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
        dumpInfo.ExceptionPointers = pException;
        dumpInfo.ThreadId = GetCurrentThreadId();
        dumpInfo.ClientPointers = FALSE;

        // 写入Dump文件内容
        MiniDumpWriteDump(
            GetCurrentProcess(),
            GetCurrentProcessId(), 
            hDumpFile,
            MiniDumpNormal,
            //MiniDumpWithFullMemoryInfo,
            &dumpInfo, NULL, NULL);
        CloseHandle(hDumpFile);
    }
}

int WINAPI WinMain(
    __in HINSTANCE hInstance,
    __in_opt HINSTANCE hPrevInstance,
    __in LPSTR lpCmdLine,
    __in int nShowCmd
    )
{
    //EnableAutoDump();
    
    HMODULE hMod = LoadLibraryW(kWkeDllPath);
    if (hMod)
    {
        FN_wkeInitializeEx wkeInitializeExFunc = (FN_wkeInitializeEx)GetProcAddress(hMod, "wkeInitializeEx");
       
        if (wkeInitializeExFunc)
        {
            MessageBoxA(NULL, "1111222", "2222111111", MB_OK);
            wkeInitializeExFunc(0);

            MessageBoxA(NULL, "1111222", "333333", MB_OK);
            WKE_FOR_EACH_DEFINE_FUNCTION(WKE_GET_PTR_ITERATOR0, WKE_GET_PTR_ITERATOR1, WKE_GET_PTR_ITERATOR2, WKE_GET_PTR_ITERATOR3, \
                WKE_GET_PTR_ITERATOR4, WKE_GET_PTR_ITERATOR5, WKE_GET_PTR_ITERATOR6, WKE_GET_PTR_ITERATOR11);
        }
    }

    //wkeInitialize();
    {
        Application app;
        RunApplication(&app);
    }
    wkeFinalize();

    return 0;
}
