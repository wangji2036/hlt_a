:: Copyright (C) 2024 Westberry Technology Corp., Ltd

:: Licensed under the Apache License, Version 2.0 (the "License");
:: you may not use this file except in compliance with the License.
:: You may obtain a copy of the License at

::     http://www.apache.org/licenses/LICENSE-2.0

:: Unless required by applicable law or agreed to in writing, software
:: distributed under the License is distributed on an "AS IS" BASIS,
:: WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
:: See the License for the specific language governing permissions and
:: limitations under the License.

@echo off
setlocal enabledelayedexpansion

set "config_file=.\config.h"
set "bootloader_folder=..\bootloader"

if not exist "%config_file%" (
    echo Error: Config file not found at %config_file%
    exit /b 1
)

set input_file=%1
set output_crc_file=none

set "temp_config_file=%TEMP%\temp_config.h"
type "%config_file%" > "%temp_config_file%"

for /f "usebackq delims=" %%a in ("%temp_config_file%") do (
    set "line=%%a"
    if "!line:~0,7!"=="#define" (
        set "line=!line:~7!"
        for /f "tokens=1,* delims== " %%b in ("!line!") do (
            set "macro_name=%%b"
            set "macro_value=%%c"

            if "!macro_value!" neq "" (
                for /f "tokens=1 delims=//" %%x in ("!macro_value!") do (
                    set "macro_value=%%x"
                )
            )

            for /l %%i in (0,1,100) do (
                if "!macro_value!" neq "" if "!macro_value:~-1!" == " " (
                    set "macro_value=!macro_value:~0,-1!"
                )
            )

            echo !macro_name! !macro_value!

            set "!macro_name!=!macro_value!"
        )
    )
)

del "%temp_config_file%"

set "temp_product=!FW_NAME!"
set "temp_product=!temp_product:"=!"
set "temp_product=!temp_product: =_!"
set "temp_version=!FW_VERSION!"
if "!temp_version:~0,2!"=="0x" set "temp_version=!temp_version:~2!"
if "!temp_version:~0,2!"=="0X" set "temp_version=!temp_version:~2!"
set output_crc_file=!temp_product!_crc_v!temp_version!
set output_merge_file=!temp_product!_merge_v!temp_version!

@echo on
python .\processing_firmware.py ".\Objects\%input_file%.hex" ".\Objects\%output_crc_file%.hex" %VENDOR_ID% %PRODUCT_ID% %FW_VERSION% %USAGE_PAGE% %USAGE_ID% %FW_ADDRESS%
python .\hex_merge.py --file1=".\Objects\%output_crc_file%.hex" --file2="%bootloader_folder%\bootloader.hex" --output=".\Objects\%output_merge_file%.hex"

@echo off

echo LOAD .\Objects\%output_merge_file%.hex INCREMENTAL > Pre_Download.ini

COPY /Y ".\Objects\%output_crc_file%.hex" "..\releases\"
COPY /Y ".\Objects\%output_merge_file%.hex" "..\releases\"

endlocal
