@echo off
REM Place somethingcool.exe and steam_api.dll in this folder, then run this script.
cd /d "%~dp0"

if not exist somethingcool.exe (
  echo ERROR: somethingcool.exe not found. Build it on Windows and place it here.
  pause
  exit /b 1
)

if not exist steam_api.dll (
  echo ERROR: steam_api.dll not found.
  echo Copy the Steam redistributable next to somethingcool.exe, or build the Windows stub entry point instead.
  pause
  exit /b 1
)

start "" /wait "%~dp0somethingcool.exe"
