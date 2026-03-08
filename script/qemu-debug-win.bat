@echo off
setlocal

set SCRIPT_DIR=%~dp0
for %%I in ("%SCRIPT_DIR%..") do set PROJECT_DIR=%%~fI
set IMAGE_DIR=%PROJECT_DIR%\image

call "%SCRIPT_DIR%img-write-win.bat"
if errorlevel 1 exit /b 1

start qemu-system-i386 -m 128M -s -S -drive file="%IMAGE_DIR%\disk1.img",index=0,media=disk,format=raw -drive file="%IMAGE_DIR%\disk2.img",index=1,media=disk,format=raw -d pcall,page,mmu,cpu_reset,guest_errors,page,trace:ps2_keyboard_set_translation
