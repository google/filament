# Building Filament for Android on Windows

## Prerequisites

In addition to the requirements for [building Filament on Windows](../BUILDING.md#windows), you'll
need the Android SDK and NDK. See [Getting Started with the
NDK](https://developer.android.com/ndk/guides/) for detailed installation instructions.

You'll also need [Ninja 1.8](https://github.com/ninja-build/ninja/wiki/Pre-built-Ninja-packages) (or
more recent) and [Git for Windows](https://git-scm.com/download/win) to clone the repository and run
Bash scripts.

Ensure the `%ANDROID_HOME%` environment variable is set to your Android SDK installation location.

On Windows, we require VS2019 for building the host tools. All of the following commands should be
executed in a *Visual Studio x64 Native Tools Command Prompt for VS 2019*.

### A Note About Python 3

Python 3 is required. If CMake errors because it cannot find Python 3:

```
Could NOT find PythonInterp: Found unsuitable version "1.4", but required is at least "3"
```

then add the following flag to the CMake invocations:

```
-DPYTHON_EXECUTABLE:FILEPATH=\path\to\python3
```

## Desktop Tools

First, a few Filament tools need to be compiled for desktop.

1. From Filament's root directory, create a desktop build directory and run CMake.

```
mkdir out\cmake-release
cd out\cmake-release
cmake ^
    -G Ninja ^
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

If you are going to build Filament samples you should install desktop host tools:

```
ninja install
```

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

The Gradle project used to generate the AAR is located at `<filament>\android`.

```
cd android
gradlew -Pcom.google.android.filament.dist-dir=..\out\android-release\filament assembleRelease
copy filament-android\build\outputs\aar\filament-android-release.aar ..\..\out\
```

If you're only interested in building for a single ABI, you'll need to pass a `com.google.android.filament.abis` parameter:

```
gradlew -Pcom.google.android.filament.dist-dir=..\out\android-release\filament assembleRelease -Pcom.google.android.filament.abis=x86
```

If you're only interested in building SDK, you may skip samples build by passing a `com.google.android.filament.skip-samples` flag:

```
gradlew -Pcom.google.android.filament.dist-dir=..\out\android-release\filament assembleRelease -Pcom.google.android.filament.skip-samples
```


`filament-android-release.aar` should now be present at `<filament>\out\filament-android-release.aar`.

See [Using Filament's AAR](../README.md) for usage instructions.

