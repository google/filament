# Using External Benchmarks with ANGLE

This document contains instructions on how to run external benchmarks on ANGLE as the GLES renderer.
There is a section for each benchmark with subsections for each platform.  The general theme is to
make the benchmark application pick ANGLE's `libGLESv2.so` and `libEGL.so` files instead of the
system ones.

On Linux, this is generally achieved with setting `LD_LIBRARY_PATH`.  On Windows, ANGLE dlls may
need to be copied to the benchmark's executable directory.

## glmark2

This benchmark can be found on [github](https://github.com/glmark2/glmark2).  It's written against
GLES 2.0 and supports Linux and Android.  It performs tens of tests and reports the framerate for
each test.

### glmark2 on Linux

To build glmark2 on Linux:

```
$ git clone https://github.com/glmark2/glmark2.git
$ cd glmark2
$ ./waf configure --with-flavors=x11-glesv2 --data-path=$PWD/data/
$ ./waf
```

To run glmark2 using the native implementation of GLES:

```
$ cd build/src
$ ./glmark2-es2
```

To run glmark2 using ANGLE, we need to first create a few links in the build directory of ANGLE:

```
$ cd /path/to/angle/out/release
$ ln -s libEGL.so libEGL.so.1
$ ln -s libGLESv2.so libGLESv2.so.2
```

Back in glmark2, we need to make sure these shared objects are picked up:

```
$ cd /path/to/glmark2/build/src
$ LD_LIBRARY_PATH=/path/to/angle/out/release/ ldd ./glmark2-es2
```

With `ldd`, you can verify that `libEGL.so.1` and `libGLESv2.so.2` are correctly picked up from
ANGLE's build directory.

To run glmark2 on the default back-end of ANGLE:

```
$ LD_LIBRARY_PATH=/path/to/angle/out/release/ ./glmark2-es2
```

To run glmark2 on a specific back-end of ANGLE:

```
$ ANGLE_DEFAULT_PLATFORM=vulkan LD_LIBRARY_PATH=/path/to/angle/out/release/ ./glmark2-es2
```

### glmark2 on Linux for Android

**Prerequisites**

Below steps are set up to use version 26.0.1 of build-tools, which can be downloaded here:

[https://dl.google.com/android/repository/build-tools_r26.0.1-linux.zip](https://dl.google.com/android/repository/build-tools_r26.0.1-linux.zip)

Tested with r19 of NDK, which can be downloaded here:

[https://dl.google.com/android/repository/android-ndk-r19-linux-x86_64.zip](https://dl.google.com/android/repository/android-ndk-r19-linux-x86_64.zip)

Tested with OpenJDK 8:

```
sudo apt-get install openjdk-8-jdk
```

Note: This is built from a branch that has fixes for Android.  It only supports
32-bit ARM (armeabi-v7a).  Supporting other ABIs requires more work, possibly
including a move to cmake instead of ndk-build.

**Setup**

```
export ANDROID_SDK=<path_to_Android_SDK>
export ANDROID_NDK=<path_to_Android_NDK>
export JAVA_HOME=/usr/lib/jvm/java-8-openjdk-amd64
```

**Build**

```
git clone https://github.com/cnorthrop/glmark2.git
cd glmark2/android
git checkout android_fixes
./build.sh
```

**Install**

```
adb install --abi armeabi-v7a glmark2.apk
```

**Run**

To select ANGLE as the driver on Android (requires Android Q):

```
adb shell settings put global angle_gl_driver_selection_pkgs org.linaro.glmark2
adb shell settings put global angle_gl_driver_selection_values angle
```

To switch back to native GLES driver:

```
adb shell settings delete global angle_gl_driver_selection_values
adb shell settings delete global angle_gl_driver_selection_pkgs
```
