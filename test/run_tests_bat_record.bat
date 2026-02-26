@echo off
setlocal

set GCC=C:\msys64\ucrt64\bin\gcc.exe
set PROJ_ROOT=%~dp0..

if not exist %GCC% (
    echo ERROR: MSYS2 GCC not found at %GCC%
    echo Please install MSYS2 and the mingw-w64-ucrt-x86_64-gcc package.
    echo   pacman -S mingw-w64-ucrt-x86_64-gcc
    exit /b 1
)

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
    echo.
    echo Expected failure: test_temperature_onset_uses_ntc_to_temp
    echo   This test is designed to FAIL on current bat_record.c.
    echo   It confirms the Phase 1 bug: gd->sys_infos.ntc_temp_wpc is used
    echo   instead of calling ntc_to_temp(ntc_resistance).
    echo   The test will PASS after Phase 3 applies the fix.
)

exit /b %TEST_RESULT%
