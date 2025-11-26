#include "pch.h"
#include "CrashHandler.h"

#include <dbghelp.h>
#include <Shlwapi.h>

#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "Shlwapi.lib")

wchar_t FCrashHandler::DumpDirectory[MAX_PATH] = L"CrashDumps";
bool FCrashHandler::bCrashInjection = false;

std::wstring GetExceptionDescription(DWORD ExceptionCode)
{
    switch (ExceptionCode)
    {
    case EXCEPTION_ACCESS_VIOLATION:         return L"Access Violation (메모리 접근 위반)";
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:    return L"Array Bounds Exceeded (배열 범위 초과)";
    case EXCEPTION_BREAKPOINT:               return L"Breakpoint (중단점)";
    case EXCEPTION_DATATYPE_MISALIGNMENT:    return L"Datatype Misalignment (데이터 정렬 오류)";
    case EXCEPTION_FLT_DENORMAL_OPERAND:     return L"Float: Denormal Operand";
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:       return L"Float: Divide By Zero (0으로 나누기)";
    case EXCEPTION_FLT_INEXACT_RESULT:       return L"Float: Inexact Result";
    case EXCEPTION_FLT_INVALID_OPERATION:    return L"Float: Invalid Operation";
    case EXCEPTION_FLT_OVERFLOW:             return L"Float: Overflow";
    case EXCEPTION_FLT_STACK_CHECK:          return L"Float: Stack Check";
    case EXCEPTION_FLT_UNDERFLOW:            return L"Float: Underflow";
    case EXCEPTION_ILLEGAL_INSTRUCTION:      return L"Illegal Instruction (잘못된 명령어)";
    case EXCEPTION_IN_PAGE_ERROR:            return L"In Page Error (페이지 오류)";
    case EXCEPTION_INT_DIVIDE_BY_ZERO:       return L"Integer: Divide By Zero (0으로 나누기)";
    case EXCEPTION_INT_OVERFLOW:             return L"Integer: Overflow";
    case EXCEPTION_INVALID_DISPOSITION:      return L"Invalid Disposition";
    case EXCEPTION_NONCONTINUABLE_EXCEPTION: return L"Noncontinuable Exception";
    case EXCEPTION_PRIV_INSTRUCTION:         return L"Privileged Instruction";
    case EXCEPTION_STACK_OVERFLOW:           return L"Stack Overflow (스택 오버플로우)";
    default: return L"Unknown Exception";
    }
}

void FCrashHandler::Init()
{
    //  폴더가 없으면 생성
    CreateDirectoryW(DumpDirectory, nullptr);
    // 전역 Unhandled Exception 필터 등록
    ::SetUnhandledExceptionFilter(&FCrashHandler::UnhandledExceptionFilter);
}

LONG __stdcall FCrashHandler::UnhandledExceptionFilter(_EXCEPTION_POINTERS* ExceptionInfo)
{
    // 1. 시간 문자열 생성
    auto Now = std::chrono::system_clock::now();
    auto InTimeT = std::chrono::system_clock::to_time_t(Now);

    std::wstringstream TimeStream;
    struct tm TimeInfo;
    errno_t err = localtime_s(&TimeInfo, &InTimeT);
    if (err == 0) TimeStream << std::put_time(&TimeInfo, L"%Y-%m-%d_%H-%M-%S");
    else TimeStream << L"UnknownTime";
    std::wstring TimeString = TimeStream.str();

    // 2. 로그 파일 경로
    std::wstring LogFileName = L"Mundi_CrashLog_" + TimeString + L".txt";
    std::wstring FullLogPath = std::wstring(DumpDirectory) + L"\\" + LogFileName;

    // ============================================================
    // 알림창에 띄울 핵심 정보를 담을 변수들
    // ============================================================
    std::wstring CrashFunc = L"Unknown Function";
    std::wstring CrashFile = L"Unknown File";
    int CrashLine = 0;
    bool bCapturedTopFrame = false; // 맨 위 스택을 잡았는지 체크

    // ============================================================
    // 심볼 초기화 및 로그 작성
    // ============================================================
    HANDLE hProcess = GetCurrentProcess();
    HANDLE hThread = GetCurrentThread();
    std::string SearchPath = GetSymbolSearchPath();

    SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_DEBUG);
    SymInitialize(hProcess, SearchPath.c_str(), TRUE);

    std::wofstream LogFile(FullLogPath);
    if (LogFile.is_open())
    {
        LogFile << L"=== Crash Detected ===" << std::endl;
        LogFile << L"Time: " << TimeString << std::endl;
        LogFile << L"Exception Code: 0x" << std::hex << ExceptionInfo->ExceptionRecord->ExceptionCode << std::dec;
        LogFile << L" (" << GetExceptionDescription(ExceptionInfo->ExceptionRecord->ExceptionCode) << L")" << std::endl;
        LogFile << L"=== Call Stack ===" << std::endl;

        STACKFRAME64 StackFrame = {};
        DWORD MachineType = IMAGE_FILE_MACHINE_AMD64;
        CONTEXT Context = *ExceptionInfo->ContextRecord;

        StackFrame.AddrPC.Offset = Context.Rip;
        StackFrame.AddrPC.Mode = AddrModeFlat;
        StackFrame.AddrFrame.Offset = Context.Rbp;
        StackFrame.AddrFrame.Mode = AddrModeFlat;
        StackFrame.AddrStack.Offset = Context.Rsp;
        StackFrame.AddrStack.Mode = AddrModeFlat;

        while (StackWalk64(MachineType, hProcess, hThread, &StackFrame, &Context, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL))
        {
            if (StackFrame.AddrPC.Offset == 0) break;

            // --- 심볼 정보 가져오기 ---
            char SymbolBuffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
            PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)SymbolBuffer;
            pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
            pSymbol->MaxNameLen = MAX_SYM_NAME;

            DWORD64 Displacement = 0;
            std::string FunctionName = "(Unknown Function)";
            if (SymFromAddr(hProcess, StackFrame.AddrPC.Offset, &Displacement, pSymbol)) {
                FunctionName = pSymbol->Name;
            }

            // --- 라인 정보 가져오기 ---
            IMAGEHLP_LINE64 Line;
            Line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
            DWORD DisplacementLine = 0;
            std::string FileName = "(Unknown File)";
            int LineNumber = 0;
            if (SymGetLineFromAddr64(hProcess, StackFrame.AddrPC.Offset, &DisplacementLine, &Line)) {
                FileName = Line.FileName;
                LineNumber = Line.LineNumber;
            }

            // ============================================================
            // [핵심] 첫 번째(Top) 스택 프레임 정보 저장 (알림창용)
            // ============================================================
            if (!bCapturedTopFrame)
            {
                CrashFunc = ACPToWide(FunctionName);
                CrashFile = ACPToWide(FileName);
                CrashLine = LineNumber;
                bCapturedTopFrame = true;
            }

            // 로그 파일 기록
            LogFile << ACPToWide(FunctionName) << L" - " << ACPToWide(FileName) << L" (" << LineNumber << L")" << std::endl;
        }
        LogFile.close();
    }
    SymCleanup(hProcess);

    // ============================================================
    // 미니덤프 저장
    // ============================================================
    std::wstring DumpFileName = L"Mundi_CrashDump_" + TimeString + L".dmp";
    std::wstring FullDumpPath = std::wstring(DumpDirectory) + L"\\" + DumpFileName;
    
    HANDLE hFile = CreateFileW(FullDumpPath.c_str(), GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        _MINIDUMP_EXCEPTION_INFORMATION ExInfo;
        ExInfo.ThreadId = GetCurrentThreadId();
        ExInfo.ExceptionPointers = ExceptionInfo;
        ExInfo.ClientPointers = FALSE;
        MINIDUMP_TYPE DumpType = (MINIDUMP_TYPE)(MiniDumpNormal | MiniDumpWithDataSegs | MiniDumpWithHandleData);

        MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, DumpType, ExceptionInfo ? &ExInfo : nullptr, nullptr, nullptr);
        CloseHandle(hFile);
    }

    // ============================================================
    // [수정됨] 상세 정보를 포함한 알림창
    // ============================================================
    std::wstringstream MsgBoxStream;
    MsgBoxStream << L"[Mundi Engine Crash Report]\n\n";
    
    // 1. 에러 종류
    MsgBoxStream << L"■ 에러 내용:\n";
    MsgBoxStream << L"   " << GetExceptionDescription(ExceptionInfo->ExceptionRecord->ExceptionCode) << L"\n\n";
    
    // 2. 발생 위치 (알림창에서는 파일명만 간단히 보여주기 위해 경로 파싱)
    std::wstring SimpleFileName = CrashFile;
    size_t lastSlash = CrashFile.find_last_of(L"\\/");
    if (lastSlash != std::wstring::npos) SimpleFileName = CrashFile.substr(lastSlash + 1);

    MsgBoxStream << L"■ 발생 위치:\n";
    if (bCapturedTopFrame) {
        MsgBoxStream << L"   파일: " << SimpleFileName << L"\n";
        MsgBoxStream << L"   라인: " << CrashLine << L"\n";
        MsgBoxStream << L"   함수: " << CrashFunc << L"()\n\n";
    } else {
        MsgBoxStream << L"   (콜스택 정보를 가져오지 못했습니다)\n\n";
    }

    MsgBoxStream << L"■ 저장된 파일:\n";
    MsgBoxStream << L"   " << LogFileName << L"\n";
    MsgBoxStream << L"   " << DumpFileName;

    MessageBoxW(NULL, MsgBoxStream.str().c_str(), L"치명적인 오류 발생!", MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);

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

FString FCrashHandler::GetSymbolSearchPath()
{
    // 로컬 PDB 우선
    char ExePath[MAX_PATH];
    GetModuleFileNameA(NULL, ExePath, MAX_PATH);
    std::string LocalDir = ExePath;
    size_t LastSlash = LocalDir.find_last_of("\\/");
    if (LastSlash != std::string::npos)
    {
        LocalDir = LocalDir.substr(0, LastSlash);
    }

    // 임시 폴더(SymbolCache) 경로
    char TempPath[MAX_PATH];
    GetTempPathA(MAX_PATH, TempPath);
    std::string CacheDir = std::string(TempPath) + "SymbolCache";
    CreateDirectoryA(CacheDir.c_str(), NULL);

    // 심볼 서버 경로
    std::string ServerPath = R"(\\172.21.11.109\SymbolStore)";

    // 경로 합치기 (우선순위 순서대로 세미콜론 ; 으로 구분)
    std::stringstream ss;
    ss << LocalDir << ";"; // 1순위: 로컬
    ss << "srv*" << CacheDir << "*" << ServerPath; // 2순위: 없으면 서버에서 다운받아 캐시에 저장

    return ss.str();
}
