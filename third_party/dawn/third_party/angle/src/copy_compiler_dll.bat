@echo off
set _arch=%1
set _arch=%_arch:Win32=x86%
copy %2"\Redist\D3D\"%_arch%"\d3dcompiler_47.dll" %3 > NUL
:: This is equivalent to `touch', see:
:: https://technet.microsoft.com/en-us/library/bb490886.aspx
:: This avoids rerunning because this batch file is also copied to the gen dir,
:: so it's timestamp would otherwise be newer than the dll.
copy /b %3\d3dcompiler_47.dll+,, %3\d3dcompiler_47.dll > NUL
