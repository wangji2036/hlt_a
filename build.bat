@echo off
setlocal EnableExtensions

call :enter_project_root
if errorlevel 1 goto :fail

set "PROJ_ROOT=%CD%"
set "CONFIG=Debug"
set "TARGET_DIR=%PROJ_ROOT%\%CONFIG%"
set "NO_PAUSE="
if /I "%~1"=="--no-pause" set "NO_PAUSE=1"

call :ensure_cds
if errorlevel 1 goto :fail

set "MAKE_EXE=%CDS_HOME%\MinGW\msys\1.0\bin\make.exe"
set "GCC_EXE=%CDS_HOME%\MinGW\csky-abiv2-elf-toolchain\bin\csky-abiv2-elf-gcc.exe"

if not exist "%MAKE_EXE%" (
    echo FAIL make.exe was not found:
    echo FAIL %MAKE_EXE%
    echo DD_RESULT^|build_status^|FAIL
    echo DD_RESULT^|build_reason^|missing_make
    goto :fail
)

if not exist "%GCC_EXE%" (
    echo FAIL csky-abiv2-elf-gcc.exe was not found:
    echo FAIL %GCC_EXE%
    echo DD_RESULT^|build_status^|FAIL
    echo DD_RESULT^|build_reason^|missing_gcc
    goto :fail
)

if not exist "%TARGET_DIR%\makefile" (
    echo FAIL Build makefile was not found:
    echo FAIL %TARGET_DIR%\makefile
    echo DD_RESULT^|build_status^|FAIL
    echo DD_RESULT^|build_reason^|missing_makefile
    goto :fail
)

set "PATH=%CDS_HOME%\MinGW\csky-abiv2-elf-toolchain\bin;%CDS_HOME%\MinGW\bin;%CDS_HOME%\MinGW\msys\1.0\bin;%PATH%"
set "C_INCLUDE_PATH="
call :configure_include_path

echo info Toolchain: %CDS_HOME%
echo info Build dir : %TARGET_DIR%
echo DD_RESULT^|build_dir^|%TARGET_DIR%
echo DD_RESULT^|toolchain^|%CDS_HOME%
echo info Running make clean...
pushd "%TARGET_DIR%"
"%MAKE_EXE%" clean
if errorlevel 1 (
    popd
    echo FAIL make clean failed.
    echo DD_RESULT^|build_status^|FAIL
    echo DD_RESULT^|build_reason^|make_clean_failed
    goto :fail
)

echo info Running make all...
"%MAKE_EXE%" all
if errorlevel 1 (
    popd
    echo FAIL make all failed.
    echo DD_RESULT^|build_status^|FAIL
    echo DD_RESULT^|build_reason^|make_all_failed
    goto :fail
)
popd

echo.
echo PASS Build completed.
echo DD_RESULT^|build_status^|PASS
call :print_latest_artifact bin "BIN"
call :print_latest_artifact elf "ELF"
call :print_latest_artifact hex "HEX"
goto :success

:enter_project_root
cd /d "%~dp0"
if exist ".project" goto :eof
if exist "..\.project" (
    cd /d ".."
    goto :eof
)
echo FAIL Cannot locate project root from %~dp0
exit /b 1

:ensure_cds
set "CDS_HOME="
call :find_cds_home
if defined CDS_HOME exit /b 0

echo warn C-Sky Development Suite was not found locally.
call :find_cds_installer
if not defined CDS_INSTALLER (
    echo FAIL CDS auto-install source was not found.
    echo FAIL Put an installer at:
    echo FAIL   %PROJ_ROOT%\build_release\tools\
    echo FAIL Or set CDS_INSTALLER to a trusted local .exe or .msi path.
    exit /b 1
)

echo info Launching CDS installer: %CDS_INSTALLER%
call :run_cds_installer "%CDS_INSTALLER%"
if errorlevel 1 (
    echo FAIL CDS installer exited with failure.
    exit /b 1
)

set "CDS_HOME="
call :find_cds_home
if not defined CDS_HOME (
    echo FAIL CDS still was not detected after installation.
    exit /b 1
)
exit /b 0

:find_cds_installer
if defined CDS_INSTALLER (
    if exist "%CDS_INSTALLER%" goto :eof
    set "CDS_INSTALLER="
)

 for %%P in (
    "%PROJ_ROOT%\build_release\tools\C-Sky Development Suite*.exe"
    "%PROJ_ROOT%\build_release\tools\C-Sky Development Suite*.msi"
    "%PROJ_ROOT%\build_release\tools\cds-installer.exe"
    "%PROJ_ROOT%\build_release\tools\cds-installer.msi"
    "%PROJ_ROOT%\tools\C-Sky Development Suite*.exe"
    "%PROJ_ROOT%\tools\C-Sky Development Suite*.msi"
    "%PROJ_ROOT%\tools\cds-installer.exe"
    "%PROJ_ROOT%\tools\cds-installer.msi"
 ) do (
    if exist "%%~fP" (
        set "CDS_INSTALLER=%%~fP"
        goto :eof
    )
)
goto :eof

:run_cds_installer
set "INSTALLER_PATH=%~1"
if not exist "%INSTALLER_PATH%" (
    echo FAIL CDS installer was not found: %INSTALLER_PATH%
    exit /b 1
)

if /I "%~x1"==".msi" (
    msiexec /i "%INSTALLER_PATH%" /qn /norestart
    exit /b %ERRORLEVEL%
)

if /I "%~x1"==".exe" (
    if defined CDS_INSTALL_SILENT_ARGS (
        call "%INSTALLER_PATH%" %CDS_INSTALL_SILENT_ARGS%
        exit /b %ERRORLEVEL%
    )
    if /I "%CDS_INSTALLER_ALLOW_GUI%"=="1" (
        start /wait "" "%INSTALLER_PATH%"
        exit /b %ERRORLEVEL%
    )
    echo FAIL EXE installers require CDS_INSTALL_SILENT_ARGS or CDS_INSTALLER_ALLOW_GUI=1.
    exit /b 1
)

echo FAIL Unsupported CDS installer type: %INSTALLER_PATH%
exit /b 1

:print_latest_artifact
set "LATEST_FILE="
for /f "delims=" %%F in ('dir /b /a-d /o-d "%TARGET_DIR%\*.%~1" 2^>nul') do (
    if not defined LATEST_FILE set "LATEST_FILE=%TARGET_DIR%\%%F"
)
if defined LATEST_FILE (
    echo PASS %~2: %LATEST_FILE%
    echo DD_RESULT^|artifact_%~1^|%LATEST_FILE%
) else (
    echo DD_RESULT^|artifact_%~1^|
)
goto :eof

:configure_include_path
for %%D in (
    "%PROJ_ROOT%\app"
    "%PROJ_ROOT%\fml"
    "%PROJ_ROOT%\gauge"
    "%PROJ_ROOT%\hal"
    "%PROJ_ROOT%\inc"
    "%PROJ_ROOT%\libpowerbank"
    "%PROJ_ROOT%\osal"
    "%PROJ_ROOT%\power"
    "%PROJ_ROOT%\startup"
    "%PROJ_ROOT%\test"
    "%PROJ_ROOT%\usbpd"
    "%PROJ_ROOT%\util"
) do (
    if exist "%%~fD" call :append_include "%%~fD"
)
goto :eof

:append_include
set "CANDIDATE=%~1"
if not defined C_INCLUDE_PATH (
    set "C_INCLUDE_PATH=%CANDIDATE%"
) else (
    echo;%C_INCLUDE_PATH%; | find /I ";%CANDIDATE%;" >nul
    if errorlevel 1 set "C_INCLUDE_PATH=%C_INCLUDE_PATH%;%CANDIDATE%"
)
goto :eof

:find_cds_home
if defined CDS_HOME (
    if exist "%CDS_HOME%\MinGW\csky-abiv2-elf-toolchain\bin\csky-abiv2-elf-gcc.exe" goto :eof
    set "CDS_HOME="
)

for %%P in (
    "D:\CDS\C-Sky Development Suite"
    "C:\Program Files (x86)\C-Sky\C-Sky Development Suite"
    "C:\Program Files\C-Sky\C-Sky Development Suite"
) do (
    if exist "%%~P\MinGW\csky-abiv2-elf-toolchain\bin\csky-abiv2-elf-gcc.exe" (
        set "CDS_HOME=%%~P"
        goto :eof
    )
)
goto :eof

:fail
echo DD_RESULT^|build_status^|FAIL
set "EXIT_CODE=1"
goto :finish

:success
set "EXIT_CODE=0"

:finish
if not defined NO_PAUSE pause
exit /b %EXIT_CODE%
