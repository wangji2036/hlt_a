@echo off
setlocal EnableExtensions
chcp 65001 >nul

set "NO_PAUSE="
if /I "%~1"=="--no-pause" set "NO_PAUSE=1"

call :enter_project_root
if errorlevel 1 exit /b 1

if exist "%CD%\build_release\auto_release.bat" (
    call "%CD%\build_release\auto_release.bat" %*
    exit /b %ERRORLEVEL%
)

call :ensure_python
if errorlevel 1 (
    echo FAIL Python was not found and auto-install failed.
    echo DD_RESULT^|release_status^|FAIL
    echo DD_RESULT^|release_reason^|python_unavailable
    if not defined NO_PAUSE pause
    exit /b 1
)

set "PY_RELEASE_EXTRA="
if /I "%DD_FORCE_RELEASE%"=="1" set "PY_RELEASE_EXTRA=--force"
"%PYTHON_CMD%" auto_release.py %PY_RELEASE_EXTRA%
set "EXIT_CODE=%ERRORLEVEL%"
if not defined NO_PAUSE pause
exit /b %EXIT_CODE%

:enter_project_root
cd /d "%~dp0"
if exist ".project" goto :eof
if exist "..\.project" (
    cd /d ".."
    goto :eof
)
echo FAIL Cannot locate project root from %~dp0
echo DD_RESULT^|release_status^|FAIL
echo DD_RESULT^|release_reason^|missing_project_root
exit /b 1

:ensure_python
set "PYTHON_CMD="
call :find_python
if defined PYTHON_CMD exit /b 0

echo warn Python was not found locally.
where winget >nul 2>&1
if errorlevel 1 exit /b 1

echo info Installing Python 3.13 via winget...
winget install -e --id Python.Python.3.13 --accept-package-agreements --accept-source-agreements --disable-interactivity
if errorlevel 1 exit /b 1

set "PYTHON_CMD="
call :find_python
if defined PYTHON_CMD exit /b 0
exit /b 1

:find_python
call :try_python_candidate "python"
if defined PYTHON_CMD goto :eof
call :try_python_candidate "py"
if defined PYTHON_CMD goto :eof
call :try_python_candidate "python3"
if defined PYTHON_CMD goto :eof

for %%P in (
    "%LOCALAPPDATA%\Programs\Python\Python313\python.exe"
    "%LOCALAPPDATA%\Programs\Python\Python312\python.exe"
    "%LOCALAPPDATA%\Programs\Python\Python311\python.exe"
    "%LOCALAPPDATA%\Programs\Python\Python310\python.exe"
    "%LOCALAPPDATA%\Programs\Python\Python39\python.exe"
    "C:\Python313\python.exe"
    "C:\Python312\python.exe"
    "C:\Python311\python.exe"
    "C:\Python310\python.exe"
    "C:\Python39\python.exe"
) do (
    call :try_python_candidate "%%~fP"
    if defined PYTHON_CMD goto :eof
)
goto :eof

:try_python_candidate
if "%~1"=="" goto :eof
"%~1" --version >nul 2>&1
if errorlevel 1 goto :eof
set "PYTHON_CMD=%~1"
goto :eof
