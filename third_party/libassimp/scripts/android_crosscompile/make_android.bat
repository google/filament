@echo off

set ASSIMP_PATH=D:\projects\asset-importer-lib\assimp
set CMAKE_PATH="C:\Program Files\CMake\bin\cmake.exe"
set ANDROID_NDK_PATH=C:\Users\kimkulling\AppData\Local\Android\Sdk\ndk-bundle
set ANDROID_CMAKE_PATH=contrib\android-cmake

pushd %ASSIMP_PATH%

rmdir /s /q build
mkdir build
cd build

%CMAKE_PATH% .. ^
  -G"MinGW Makefiles" ^
  -DCMAKE_BUILD_TYPE=Release ^
  -DCMAKE_TOOLCHAIN_FILE=%ANDROID_CMAKE_PATH%\android.toolchain.cmake ^
  -DCMAKE_MAKE_PROGRAM=%ANDROID_NDK_PATH%\prebuilt\windows-x86_64\bin\make.exe ^
  -DANDROID_NDK=%ANDROID_NDK_PATH% ^
  -DANDROID_NATIVE_API_LEVEL=android-9 ^
  -DASSIMP_ANDROID_JNIIOSYSTEM=ON ^
  -DANDROID_ABI=arm64-v8a ^
  -DASSIMP_BUILD_ZLIB=ON ^
  -DASSIMP_BUILD_TESTS=OFF

%CMAKE_PATH% --build .

popd