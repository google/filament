# Building Filament for Android on Windows

## Prerequisites

In addition to the requirements for [building Filament on Windows](../README.md#windows), you'll
need the Android SDK and NDK. See [Getting Started with the
NDK](https://developer.android.com/ndk/guides/) for detailed installation instructions.

Ensure the `%ANDROID_HOME%` environment variable is set to your Android SDK installation location.

All of the following commands should be executed in a Visual Studio x64 Native Tools Command Prompt.

## Desktop Tools

First, a few Filament tools need to be compiled for desktop.

1. From Filament's root directory, create a desktop build directory and run CMake.

```
mkdir out\cmake-release
cd out\cmake-release
cmake ^
    -G Ninja ^
    -DCMAKE_CXX_COMPILER:PATH="C:\Program Files\LLVM\bin\clang-cl.exe" ^
    -DCMAKE_C_COMPILER:PATH="C:\Program Files\LLVM\bin\clang-cl.exe" ^
    -DCMAKE_LINKER:PATH="C:\Program Files\LLVM\bin\lld-link.exe" ^
    -DCMAKE_INSTALL_PREFIX=..\release\filament ^
    -DFILAMENT_ENABLE_JAVA=NO ^
    -DCMAKE_BUILD_TYPE=Release ^
    ..\..
```

2. Build the required desktop host tools.

```
ninja matc resgen cmgen
```

The build should succeed and a `ImportExecutables-Release.cmake` file should automatically be
created at Filament's root directory.

## Build

1. Create the build directories.

```
mkdir out\cmake-android-release-aarch64
mkdir out\cmake-android-release-arm7
mkdir out\cmake-android-release-x86_64
mkdir out\cmake-android-release-x86
```

2. Run CMake for each architecture.

```
cd out\cmake-android-release-aarch64
cmake ^
    -G Ninja ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_INSTALL_PREFIX=..\android-release\filament ^
    -DCMAKE_TOOLCHAIN_FILE=..\..\build\toolchain-aarch64-linux-android.cmake ^
    ..\..

cd out\cmake-android-release-arm7
cmake ^
    -G Ninja ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_INSTALL_PREFIX=..\android-release\filament ^
    -DCMAKE_TOOLCHAIN_FILE=..\..\build\toolchain-arm7-linux-android.cmake ^
    ..\..

cd out\cmake-android-release-x86_64
cmake ^
    -G Ninja ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_INSTALL_PREFIX=..\android-release\filament ^
    -DCMAKE_TOOLCHAIN_FILE=..\..\build\toolchain-x86_64-linux-android.cmake ^
    ..\..

cd out\cmake-android-release-x86
cmake ^
    -G Ninja ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_INSTALL_PREFIX=..\android-release\filament ^
    -DCMAKE_TOOLCHAIN_FILE=..\..\build\toolchain-x86-linux-android.cmake ^
    ..\..
```

3. Build.

Inside of each build directory, run:

```
ninja install
```

## Generate AAR

The Gradle project used to generate the AAR is located at `<filament>\android\filament-android`.

```
cd android\filament-android
gradlew -Pfilament_dist_dir=..\..\out\android-release\filament assembleRelease
copy build\outputs\aar\filament-android-release.aar ..\..\out\
```

If you're only interested in building for a single ABI, you'll need to add an `abiFilters` override
inside the `build.gradle` file underneath `defaultConfig`:

```
ndk {
    abiFilters 'arm64-v8a'
}
```

See
[NdkOptions](https://google.github.io/android-gradle-dsl/current/com.android.build.gradle.internal.dsl.NdkOptions.html#com.android.build.gradle.internal.dsl.NdkOptions:abiFilters)
for more information.

`filament-android-release.aar` should now be present at `<filament>\out\filament-android-release.aar`.

See [Using Filament's AAR](../README.md#using-filaments-aar) for usage instructions.

