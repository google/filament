rmdir /q /s build
mkdir build

cmake.exe -G "Visual Studio 16 2019" -A x64 -Bbuild .
