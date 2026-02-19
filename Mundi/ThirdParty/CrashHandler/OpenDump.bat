@echo off
setlocal

:: ========================================================
:: [Setup] Symbol Path (Hybrid Method)
:: ========================================================
:: 1. Local Cache Directory
set CACHE_DIR=C:\Users\%USERNAME%\AppData\Local\Temp\SymbolCache
if not exist "%CACHE_DIR%" mkdir "%CACHE_DIR%"

:: 2. Symbol Server Address
set SERVER_PATH=\\172.21.11.109\SymbolStore

:: 3. Local PDB Path (Directory containing the current dump file)
:: %~dp1 expands to the Drive and Path of the first argument (the dropped file)
set LOCAL_PATH=%~dp1

:: ========================================================
:: [Core] Set _NT_SYMBOL_PATH environment variable
:: ========================================================
:: Logic: "Check the folder containing the dump first. If not found, download from server to cache and use it."
set _NT_SYMBOL_PATH=%LOCAL_PATH%;srv*%CACHE_DIR%*%SERVER_PATH%

echo.
echo ========================================================
echo  [MundiEngine Debugger Launcher]
echo  Target Dump : %~nx1
echo  Symbol Path : %_NT_SYMBOL_PATH%
echo ========================================================
echo.
echo Launching Visual Studio...

:: Execute Dump File (Opens with default associated VS)
:: Visual Studio will inherit the _NT_SYMBOL_PATH variable set in this session.
start "" "%~1"

endlocal