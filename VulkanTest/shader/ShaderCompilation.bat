@echo off

:: 检查是否提供了输入地址
if "%~1"=="" (
    echo Please provide an input file.
    exit /b 1
)

:: 设置 glslc.exe 的路径
set GLSLC_PATH=F:\Vulkan\Bin\glslc.exe

:: 设置输出目录
set OUTPUT_DIR=F:\Code\VulkanTest\shader\

:: 获取输入文件的文件名，用于创建输出文件名
set INPUT_FILE=%~1
set FILENAME=%~n1
set EXTENSION=%~x1
set EXTENSION=%EXTENSION:.=%
set OUTPUT_PATH=%OUTPUT_DIR%%EXTENSION%.spv

:: 调用 glslc.exe，将输入文件编译到指定的输出目录
"%GLSLC_PATH%" "%INPUT_FILE%" -o "%OUTPUT_PATH%"
if errorlevel 1 (
    echo Compilation failed.
    exit /b 1
)

:: 反馈
echo.
echo Compilation completed. Output file is located at:
echo %OUTPUT_PATH%

:: 结束
exit /b 0
