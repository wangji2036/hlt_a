@echo off
REM ============================================================
REM 通用固件发布批处理脚本
REM 功能：自动查找最新固件，计算校验码，生成带版本信息的文件
REM 特点：可迁移，适用于任何工程
REM ============================================================

REM 切换到脚本所在目录（项目根目录）
cd /d "%~dp0"
if %ERRORLEVEL% neq 0 (
    echo 错误: 无法切换到项目根目录
    exit /b 1
)

REM 查找 Python 解释器
set PYTHON_CMD=

REM 尝试 python 命令
where python >nul 2>&1
if %ERRORLEVEL% == 0 (
    set PYTHON_CMD=python
    goto :found_python
)

REM 尝试 py 命令
where py >nul 2>&1
if %ERRORLEVEL% == 0 (
    set PYTHON_CMD=py
    goto :found_python
)

REM 尝试 python3 命令
where python3 >nul 2>&1
if %ERRORLEVEL% == 0 (
    set PYTHON_CMD=python3
    goto :found_python
)

REM 尝试常见的 Python 安装路径
if exist "C:\Python39\python.exe" (
    set PYTHON_CMD=C:\Python39\python.exe
    goto :found_python
)

if exist "C:\Python310\python.exe" (
    set PYTHON_CMD=C:\Python310\python.exe
    goto :found_python
)

if exist "C:\Python311\python.exe" (
    set PYTHON_CMD=C:\Python311\python.exe
    goto :found_python
)

if exist "C:\Python312\python.exe" (
    set PYTHON_CMD=C:\Python312\python.exe
    goto :found_python
)

if exist "C:\Python313\python.exe" (
    set PYTHON_CMD=C:\Python313\python.exe
    goto :found_python
)

REM 尝试从 AppData 查找
if exist "%LOCALAPPDATA%\Programs\Python\Python39\python.exe" (
    set PYTHON_CMD=%LOCALAPPDATA%\Programs\Python\Python39\python.exe
    goto :found_python
)

if exist "%LOCALAPPDATA%\Programs\Python\Python310\python.exe" (
    set PYTHON_CMD=%LOCALAPPDATA%\Programs\Python\Python310\python.exe
    goto :found_python
)

if exist "%LOCALAPPDATA%\Programs\Python\Python311\python.exe" (
    set PYTHON_CMD=%LOCALAPPDATA%\Programs\Python\Python311\python.exe
    goto :found_python
)

if exist "%LOCALAPPDATA%\Programs\Python\Python312\python.exe" (
    set PYTHON_CMD=%LOCALAPPDATA%\Programs\Python\Python312\python.exe
    goto :found_python
)

if exist "%LOCALAPPDATA%\Programs\Python\Python313\python.exe" (
    set PYTHON_CMD=%LOCALAPPDATA%\Programs\Python\Python313\python.exe
    goto :found_python
)

REM 如果都找不到，输出错误信息
echo 错误: 未找到 Python 解释器
echo 请确保 Python 已安装并添加到 PATH 环境变量中
exit /b 1

:found_python
REM 执行 Python 脚本
"%PYTHON_CMD%" auto_release.py
exit /b %ERRORLEVEL%
