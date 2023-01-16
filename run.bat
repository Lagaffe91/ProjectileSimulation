@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
msbuild -noConsoleLogger
cd x64\\Debug
cannon.exe
cd ..\\..