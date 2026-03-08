@echo off
setlocal

set SCRIPT_DIR=%~dp0
for %%I in ("%SCRIPT_DIR%..") do set PROJECT_DIR=%%~fI
set IMAGE_DIR=%PROJECT_DIR%\image
set BOOT_BIN=%IMAGE_DIR%\boot.bin
set DISK1_NAME=%IMAGE_DIR%\disk1.img
set DISK2_NAME=%IMAGE_DIR%\disk2.img

if not exist "%IMAGE_DIR%" mkdir "%IMAGE_DIR%"

if not exist "%BOOT_BIN%" (
    echo boot.bin not found in "%IMAGE_DIR%"
    echo hint: run make build first
    exit /b 1
)

if not exist "%DISK1_NAME%" fsutil file createnew "%DISK1_NAME%" 16777216 >nul
if not exist "%DISK2_NAME%" fsutil file createnew "%DISK2_NAME%" 33554432 >nul

dd if="%BOOT_BIN%" of="%DISK1_NAME%" bs=512 conv=notrunc count=1
