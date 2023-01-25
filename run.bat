@echo off
cl.exe 2>nul
IF %ERRORLEVEL% EQU 9009 (call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64)
call build.bat
cd x64\\Debug
IF %SUCCESS% EQU 0 cannon.exe
cd ..\\..
