@echo off
cl.exe 2>nul
IF %ERRORLEVEL% EQU 9009 (call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64)
msbuild -noConsoleLogger -m
cd x64\\Debug
cannon.exe
cd ..\\..