:: /MT flag version is built by default. Move these libs into a separate directory.
mkdir cmake-release\install\lib\x86_64\mt
move cmake-release\install\lib\x86_64\*.lib cmake-release\install\lib\x86_64\mt\

:: /MD flag.
mkdir cmake-release-md
cd cmake-release-md

cmake ..\.. -G Ninja ^
    -DCMAKE_CXX_COMPILER:PATH="clang-cl.exe" ^
    -DCMAKE_C_COMPILER:PATH="clang-cl.exe" ^
    -DCMAKE_LINKER:PATH="lld-link.exe" ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DUSE_STATIC_CRT=OFF
if errorlevel 1 exit /b %errorlevel%

ninja install
if errorlevel 1 exit /b %errorlevel%

xcopy .\install\lib\x86_64\*.lib ..\cmake-release\install\lib\x86_64\md\

cd ..

:: Debug builds

:: /MTd flag.

mkdir cmake-release-mtd
cd cmake-release-mtd

cmake ..\.. -G Ninja ^
    -DCMAKE_CXX_COMPILER:PATH="clang-cl.exe" ^
    -DCMAKE_C_COMPILER:PATH="clang-cl.exe" ^
    -DCMAKE_LINKER:PATH="lld-link.exe" ^
    -DCMAKE_BUILD_TYPE=Debug ^
    -DUSE_STATIC_CRT=ON
if errorlevel 1 exit /b %errorlevel%

ninja install
if errorlevel 1 exit /b %errorlevel%

xcopy .\install\lib\x86_64\*.lib ..\cmake-release\install\lib\x86_64\mtd\

cd ..

:: /MDd flag.

mkdir cmake-release-mdd
cd cmake-release-mdd

cmake ..\.. -G Ninja ^
    -DCMAKE_CXX_COMPILER:PATH="clang-cl.exe" ^
    -DCMAKE_C_COMPILER:PATH="clang-cl.exe" ^
    -DCMAKE_LINKER:PATH="lld-link.exe" ^
    -DCMAKE_BUILD_TYPE=Debug ^
    -DUSE_STATIC_CRT=OFF
if errorlevel 1 exit /b %errorlevel%

ninja install
if errorlevel 1 exit /b %errorlevel%

xcopy .\install\lib\x86_64\*.lib ..\cmake-release\install\lib\x86_64\mdd\

cd ..

exit /b 0
