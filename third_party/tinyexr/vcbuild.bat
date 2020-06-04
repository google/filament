rem Run
rem
rem   > python.py kuroga.py config-msvc.py
rem
rem before to generate build.ninja
rem

chcp 437
rem call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x86_amd64
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x86_amd64
ninja
