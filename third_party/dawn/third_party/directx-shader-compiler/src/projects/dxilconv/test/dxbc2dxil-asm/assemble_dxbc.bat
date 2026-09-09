@echo off

set TESTASM=%_NTTREE%\nttest\Windowstest\graphics\d3d\support\testasm.exe

FOR %%f IN (call2.asm cs3.asm  cyclecounter.asm hs3.asm indexabletemp4.asm) DO (
    %TESTASM% %%f /Fo %%~nf.dxbc
)

FOR %%f IN (indexabletemp6.asm) DO (
    %TESTASM% %%f /allowMinimumPrecision /Fo %%~nf.dxbc
)


