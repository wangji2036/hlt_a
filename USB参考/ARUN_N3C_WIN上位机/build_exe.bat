@echo off
chcp 65001 >nul
cd /d "%~dp0"

set PYTHON=C:\Users\ji.wang\AppData\Local\Programs\Python\Python313\python.exe
set UI_SRC=..\ARUN_N3C_UI设计\UI

%PYTHON% -m PyInstaller ^
  --onefile ^
  --windowed ^
  --name "ARUN充电宝智能监测系统" ^
  --distpath "dist" ^
  --workpath "build" ^
  --add-data "%UI_SRC%\logo.png;UI" ^
  --add-data "%UI_SRC%\环形电量显示.png;UI" ^
  --add-data "%UI_SRC%\电芯温度框.png;UI" ^
  --add-data "%UI_SRC%\电芯电压框.png;UI" ^
  --add-data "%UI_SRC%\设备基本信息框.png;UI" ^
  --add-data "%UI_SRC%\异常监督记录框.png;UI" ^
  --add-data "%UI_SRC%\红色电池图标.png;UI" ^
  --add-data "%UI_SRC%\蓝色电池图标.png;UI" ^
  --add-data "%UI_SRC%\Font-OPPOSans\OPPOSans-M.ttf;UI\Font-OPPOSans" ^
  --add-data "%UI_SRC%\Font-OPPOSans\OPPOSans-B.ttf;UI\Font-OPPOSans" ^
  --add-data "%UI_SRC%\Font-OPPOSans\OPPOSans-R.ttf;UI\Font-OPPOSans" ^
  --hidden-import hid ^
  --hidden-import PIL ^
  --hidden-import PIL.Image ^
  --hidden-import PIL.ImageTk ^
  --noconfirm ^
  battery_monitor.py

if %ERRORLEVEL% == 0 (
    echo.
    echo ========================================
    echo  打包成功！
    echo  输出: dist\ARUN充电宝智能监测系统.exe
    echo ========================================
) else (
    echo.
    echo 打包失败，请查看上方错误信息
)
pause
