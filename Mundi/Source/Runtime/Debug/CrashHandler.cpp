#include "pch.h"
#include "CrashHandler.h"

#include <dbghelp.h>
#include <Shlwapi.h>

#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "Shlwapi.lib")

wchar_t FCrashHandler::DumpDirectory[MAX_PATH] = L"CrashDumps";
bool FCrashHandler::bCrashInjection = false;

void FCrashHandler::Init()
{
    //  폴더가 없으면 생성
    CreateDirectoryW(DumpDirectory, nullptr);
    // 전역 Unhandled Exception 필터 등록
    ::SetUnhandledExceptionFilter(&FCrashHandler::UnhandledExceptionFilter);
}

LONG __stdcall FCrashHandler::UnhandledExceptionFilter(_EXCEPTION_POINTERS* ExceptionInfo)
{
    auto Now = std::chrono::system_clock::now();
    auto InTimeT = std::chrono::system_clock::to_time_t(Now);

    std::wstringstream ss;
    struct tm TimeInfo;
    errno_t err = localtime_s(&TimeInfo, &InTimeT);

    if (err == 0)
    {
        ss << L"Mundi_CrashDump_";
        ss << std::put_time(&TimeInfo, L"%Y-%m-%d_%H-%M-%S");
        ss << L".dmp";
    }
    else
    {
        ss << L"CrashDump_UnknownTime.dmp";
    }

    std::wstring DumpFileName = ss.str();
    std::wstring FullPath = std::wstring(DumpDirectory) + L"\\" + DumpFileName;
    
    HANDLE hFile = CreateFileW(FullPath.c_str(),
        GENERIC_WRITE, FILE_SHARE_READ, nullptr,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr
    );

    if (hFile != INVALID_HANDLE_VALUE)
    {
        _MINIDUMP_EXCEPTION_INFORMATION ExInfo;
        ExInfo.ThreadId = GetCurrentThreadId();
        ExInfo.ExceptionPointers = ExceptionInfo;
        ExInfo.ClientPointers = FALSE;
        MINIDUMP_TYPE DumpType = (MINIDUMP_TYPE)(MiniDumpNormal | MiniDumpWithDataSegs | MiniDumpWithHandleData);

        MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
            hFile, DumpType, ExceptionInfo ? &ExInfo : nullptr, nullptr, nullptr);

        CloseHandle(hFile);
    }
    
    // 예외처리했고, 프로세스 종료하라는 의미 
    return EXCEPTION_EXECUTE_HANDLER;
}

void FCrashHandler::InjectCrash()
{
    bCrashInjection = true;
}

void FCrashHandler::Crash()
{
    if (!bCrashInjection) { return; }
    TArray<UObject*>& ObjectArray = GUObjectArray;
    if (ObjectArray.Num() == 0) return;

    bool bCrashInjected = false;
    while (!bCrashInjected)
    {
        int32 RandomIndex = rand() % ObjectArray.Num();
        UObject* Victim = ObjectArray[RandomIndex];

        void** VTablePtr = reinterpret_cast<void**>(Victim);
        if (*VTablePtr == reinterpret_cast<void*>(0xDEADBEEFDEADBEEF))
        {
            continue;
        }

        bCrashInjected = true;
        *VTablePtr = reinterpret_cast<void*>(0xDEADBEEFDEADBEEF);
    }
}
