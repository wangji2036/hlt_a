@echo off
setlocal EnableExtensions
chcp 65001 >nul

set "NO_PAUSE="
if /I "%~1"=="--no-pause" set "NO_PAUSE=1"

call :enter_project_root
if errorlevel 1 (
    if not defined NO_PAUSE pause
    exit /b 1
)

if exist "%CD%\build_release\一键编译并生成Release.bat" (
    call "%CD%\build_release\一键编译并生成Release.bat" %*
    exit /b %ERRORLEVEL%
)

set "TARGET_DIR=%CD%\Debug"

echo.
echo ============================================================
echo  Build + Release
echo ============================================================
echo.

echo [Step 1/2] Build
call build.bat --no-pause
if errorlevel 1 (
    echo.
    echo [FAIL] Build failed. Release step was skipped.
    echo.
    if not defined NO_PAUSE pause
    exit /b 1
)

echo.
echo [Step 2/2] Release
set "DD_FORCE_RELEASE=1"
call auto_release.bat --no-pause
if errorlevel 1 (
    echo.
    echo [FAIL] Release generation failed.
    echo.
    if not defined NO_PAUSE pause
    exit /b 1
)

echo.
echo ============================================================
echo  Completed
call :print_latest_artifact bin "Build output"
echo  - Release root: %CD%\release
echo ============================================================
echo.
if not defined NO_PAUSE pause
exit /b 0

:enter_project_root
cd /d "%~dp0"
if exist ".project" goto :eof
if exist "..\.project" (
    cd /d ".."
    goto :eof
)
echo [ERROR] Cannot locate project root from %~dp0
exit /b 1

:print_latest_artifact
set "LATEST_FILE="
for /f "delims=" %%F in ('dir /b /a-d /o-d "%TARGET_DIR%\*.%~1" 2^>nul') do (
    if not defined LATEST_FILE set "LATEST_FILE=%TARGET_DIR%\%%F"
)
if defined LATEST_FILE (
    echo  - %~2: %LATEST_FILE%
) else (
    echo  - %~2: no .%~1 artifact found
)
goto :eof
