@echo off
rem Builds bin\LayoutReset.exe with the MSVC compiler.
setlocal
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSDIR=%%i"
if not defined VSDIR (
  echo Visual Studio with C++ build tools not found.
  exit /b 1
)
set "PATH=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer;%PATH%"
call "%VSDIR%\VC\Auxiliary\Build\vcvars64.bat" >nul || exit /b 1

cd /d "%~dp0"
if not exist bin mkdir bin
if not exist obj mkdir obj
rc /nologo /fo obj\app.res src\app.rc || exit /b 1
cl /nologo /O2 /W4 /MT /Foobj\ /Fe:bin\LayoutReset.exe src\main.c obj\app.res ^
   /link /SUBSYSTEM:WINDOWS user32.lib shell32.lib || exit /b 1
echo Built bin\LayoutReset.exe
