@rem MinGW build test - used to test MinGW builds locally
@rem Adapt path/versions before use

@rem This batch file must be used from the repository root
@if exist mingw.bat cd ..


@set PATH=%ProgramFiles%\mingw-w64\i686-4.9.2-win32-dwarf-rt_v3-rev1\mingw32\bin;%PATH%
@set PATH=%ProgramFiles%\GnuWin32\bin;%PATH%

@rem Alternative ways to use mingw
@rem make CC=gcc CFLAGS=-w CFLAGS+=-Iinclude/ CFLAGS+=-lws2_32 CFLAGS+=-liphlpapi
@rem gcc src\civetweb.c src\main.c -Iinclude\ -lws2_32 -lpthread -lcomdlg32 -w

make build CC=gcc WITH_LUA=1 WITH_WEBSOCKET=1
