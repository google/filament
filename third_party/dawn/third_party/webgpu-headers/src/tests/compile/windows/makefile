# Makefile for Microsoft NMAKE

# These target names are not real files so they will always get rebuilt
# (they are nmake "pseudotargets").
all: build-c build-cpp

clean:
	rm -f main-c.exe main-cpp.exe main.obj

MSVC_WARNINGS = /Wall /WX /options:strict /wd4820 /wd4464 /wd4189 /wd4101

build-c: ../main.c ../main.inl ../../../webgpu.h Makefile
	cl.exe /Fe:main-c.exe ../main.c /std:c11 $(MSVC_WARNINGS)
	cl.exe /Fe:main-c.exe ../main.c /std:c17 $(MSVC_WARNINGS)
	cl.exe /Fe:main-c.exe ../main.c $(MSVC_WARNINGS)
#	Docs say /std:clatest should work but it does not in VS2022.
#	cl.exe /Fe:main-c.exe ../main.c /std:clatest $(MSVC_WARNINGS)

build-cpp: ../main.cpp ../main.inl ../../../webgpu.h Makefile
	cl.exe /Fe:main-cpp.exe ../main.cpp /std:c++14 $(MSVC_WARNINGS)
	cl.exe /Fe:main-cpp.exe ../main.cpp /std:c++17 $(MSVC_WARNINGS)
	cl.exe /Fe:main-cpp.exe ../main.cpp /std:c++20 $(MSVC_WARNINGS)
	cl.exe /Fe:main-cpp.exe ../main.cpp /std:c++latest $(MSVC_WARNINGS)
