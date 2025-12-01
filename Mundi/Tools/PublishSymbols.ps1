@echo off
setlocal enabledelayedexpansion

:: ========================================================
set CONFIG=%1
if "%CONFIG%"=="" (
    echo [SymStore] Error: Configuration argument missing.
    exit /b 0
)

set TOOL_DIR=%~dp0SymStore
set BIN_DIR=%~dp0..\..\Binaries\%CONFIG%
set SERVER_IP=172.21.11.109
set STORE_PATH=\\%SERVER_IP%\SymbolStore

:: ========================================================
set GIT_HASH=NoGit
set GIT_URL=Unknown

:: Git Hash
for /f "delims=" %%i in ('git rev-parse --short HEAD') do set GIT_HASH=%%i
:: Git URL
for /f "delims=" %%i in ('git remote get-url origin') do set GIT_URL_RAW=%%i
set GIT_URL=%GIT_URL_RAW:.git=%

echo [SymStore] Target: %CONFIG% - Commit: %GIT_HASH%

:: ========================================================
echo [SymStore] Checking server availability...

powershell -Command "$t = New-Object System.Net.Sockets.TcpClient; $r = $t.BeginConnect('%SERVER_IP%', 445, $null, $null); if ($r.AsyncWaitHandle.WaitOne(3000)) { exit 0 } else { exit 1 }"

if errorlevel 1 (
    echo [Warning] Server connection timeout or unreachable.
    echo Target: %SERVER_IP%
    echo Skipping symbol upload.
    exit /b 0
)

if not exist "%STORE_PATH%" (
    echo [Warning] Server is reachable, but path not found: "%STORE_PATH%"
    echo Skipping symbol upload.
    exit /b 0
)

echo [SymStore] Server is accessible. Starting upload...

:: ========================================================
set "NOW_TIME=%time: =0%"
set VERSION_TAG=Build_%CONFIG%_%GIT_HASH%_%date:~0,4%-%date:~5,2%-%date:~8,2%_%NOW_TIME:~0,2%%NOW_TIME:~3,2%
set COMMENT="Git: %GIT_URL% (%GIT_HASH%)"

:: PDB, EXE, DLL
"%TOOL_DIR%\symstore.exe" add /f "%BIN_DIR%\*.pdb" /s "%STORE_PATH%" /t "MundiEngine" /v "%VERSION_TAG%" /c %COMMENT% /r /o
"%TOOL_DIR%\symstore.exe" add /f "%BIN_DIR%\*.exe" /s "%STORE_PATH%" /t "MundiEngine" /v "%VERSION_TAG%" /c %COMMENT% /r /o
"%TOOL_DIR%\symstore.exe" add /f "%BIN_DIR%\*.dll" /s "%STORE_PATH%" /t "MundiEngine" /v "%VERSION_TAG%" /c %COMMENT% /r /o

echo.
echo [Success] Uploaded symbols for %CONFIG% version.