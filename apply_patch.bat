@echo off
setlocal
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0patch_stm32_config.ps1"
if errorlevel 1 exit /b %errorlevel%
echo.
echo Finished.
