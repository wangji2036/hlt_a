@echo off
setlocal

set GCC=C:\msys64\ucrt64\bin\gcc.exe
set PROJ_ROOT=%~dp0..

if not exist %GCC% (
    for /f "delims=" %%I in ('where gcc 2^>nul') do (
        set GCC=%%I
        goto :gcc_found
    )
    echo ERROR: GCC not found.
    echo Checked: C:\msys64\ucrt64\bin\gcc.exe and PATH.
    echo Please install MSYS2 and the mingw-w64-ucrt-x86_64-gcc package.
    echo   pacman -S mingw-w64-ucrt-x86_64-gcc
    exit /b 1
)

:gcc_found

echo ========================================================
echo   Building test_bat_record...
echo ========================================================

:: Compile from the test/ directory
:: -I. and -Imocks: find mock headers and stub headers in mocks/
:: -I../app:        bat_record.h includes config.h (found in mocks/config.h stub)
:: -I../fml:        g_data.h is in mocks/g_data.h stub (real one guarded)
:: -I../hal:        bat_record.c includes ../hal/regdef.h (real one has REGDEF_H_ guard)
:: -Wno-int-to-pointer-cast: bat_record.c casts uint32_t->ptr (only in static fns)
:: -Wno-unused-function: mock_all.h has static inline stubs not used by all tests

%GCC% -std=c99 ^
      -Wall -Wextra ^
      -Wno-int-to-pointer-cast ^
      -Wno-unused-parameter ^
      -Wno-unused-function ^
      -Wno-unused-variable ^
      -Wno-missing-braces ^
      -I. -Imocks -I../app -I../fml -I../hal ^
      -o test_bat_record.exe test_bat_record.c

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo BUILD FAILED
    exit /b 1
)

echo Build SUCCESS
echo.
echo ========================================================
echo   Running tests...
echo ========================================================
echo.

test_bat_record.exe
set TEST_RESULT=%ERRORLEVEL%

echo.
if %TEST_RESULT% EQU 0 (
    echo All tests PASSED.
) else (
    echo Some tests FAILED.
)

exit /b %TEST_RESULT%
