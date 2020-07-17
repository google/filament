@REM Build for Visual Studio compiler. Run your copy of vcvars32.bat or vcvarsall.bat to setup command-line compiler.
set OUT_DIR=Debug
set OUT_EXE=example_sdl_opengl2
set INCLUDES=/I.. /I..\.. /I%SDL2_DIR%\include
set SOURCES=main.cpp ..\imgui_impl_sdl.cpp ..\imgui_impl_opengl2.cpp ..\..\imgui*.cpp
set LIBS=/libpath:%SDL2_DIR%\lib\x86 SDL2.lib SDL2main.lib opengl32.lib
mkdir %OUT_DIR%
cl /nologo /Zi /MD %INCLUDES% %SOURCES% /Fe%OUT_DIR%/%OUT_EXE%.exe /Fo%OUT_DIR%/ /link %LIBS% /subsystem:console
