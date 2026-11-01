@echo off
cd /d "%~dp0"
set "EDITOR_EXE=%~dp0build-vs\Debug\scene_editor.exe"
if not exist "%EDITOR_EXE%" set "EDITOR_EXE=%~dp0build-vs\Release\scene_editor.exe"
if not exist "%EDITOR_EXE%" set "EDITOR_EXE=%~dp0build\Debug\scene_editor.exe"
if not exist "%EDITOR_EXE%" set "EDITOR_EXE=%~dp0build\Release\scene_editor.exe"
if not exist "%EDITOR_EXE%" set "EDITOR_EXE=%~dp0cmake-build-debug\scene_editor.exe"
if not exist "%EDITOR_EXE%" (
  echo Editor executable not found. Build the scene_editor target first.
  pause
  exit /b 1
)
"%EDITOR_EXE%"
if errorlevel 1 (
  echo Editor exited with an error. Check the message above.
  pause
)
