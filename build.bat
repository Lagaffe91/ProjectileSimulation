@echo off
cl.exe 2>nul >nul

IF %ERRORLEVEL% EQU 9009 (call setup.bat)

mkdir x64
mkdir x64\Debug

CL.exe /MP /Iexternals/include /ZI /JMC /nologo /W3 /WX- /diagnostics:column /sdl /Od /D _DEBUG /D _CONSOLE /D _UNICODE /D UNICODE /Gm- /EHsc /RTC1 /MDd /GS /fp:precise /permissive- /Zc:wchar_t /Zc:forScope /Zc:inline /Fo"x64\Debug\\" /Fd"x64\Debug\vc142.pdb" /external:W3 /Gd /TP /FC /errorReport:queue externals\src\imgui.cpp externals\src\imgui_demo.cpp externals\src\imgui_draw.cpp externals\src\imgui_impl_glfw.cpp externals\src\imgui_impl_opengl3.cpp externals\src\imgui_tables.cpp externals\src\imgui_widgets.cpp externals\src\stb_image.cpp src\app.cpp src\cannon.cpp src\imgui_utils.cpp src\main.cpp /link  glfw3.lib opengl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /LIBPATH:"externals/libs/x86_64-w64-vc2022" /OUT:x64\Debug\cannon.exe

set /a "SUCCESS=%ERRORLEVEL%"