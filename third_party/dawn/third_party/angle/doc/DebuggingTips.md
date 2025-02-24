# Debugging Tips

There are many ways to debug ANGLE using generic or platform-dependent tools. Here is a list of tips
on how to use them.

## Running ANGLE under apitrace on Linux

[Apitrace](http://apitrace.github.io/) captures traces of OpenGL commands for later analysis,
allowing us to see how ANGLE translates OpenGL ES commands. In order to capture the trace, it
inserts a driver shim using `LD_PRELOAD` that records the command and then forwards it to the OpenGL
driver.

The problem with ANGLE is that it exposes the same symbols as the OpenGL driver so apitrace captures
the entry point calls intended for ANGLE and reroutes them to the OpenGL driver. In order to avoid
this problem, use the following:

1. Link your application against the static ANGLE libraries (libGLESv2_static and libEGL_static) so
   they don't get shadowed by apitrace's shim.
2. Ask apitrace to explicitly load the driver instead of using a dlsym on the current module.
   Otherwise apitrace will use ANGLE's symbols as the OpenGL driver entrypoint (causing infinite
   recursion). To do this you must point an environment variable to your GL driver.  For example:
   `export TRACE_LIBGL=/usr/lib/libGL.so.1`. You can find your libGL with
   `ldconfig -p | grep libGL`.
3. Link ANGLE against libGL instead of dlsyming the symbols at runtime; otherwise ANGLE won't use
   the replaced driver entry points. This is done with the gn arg `angle_link_glx = true`.

If you follow these steps, apitrace will work correctly aside from a few minor bugs like not being
able to figure out what the default framebuffer size is if there is no glViewport command.

For example, to trace a run of `hello_triangle`, assuming the apitrace executables are in `$PATH`:

```
gn args out/Debug # add "angle_link_glx = true"
# edit samples/BUILD.gn and append "_static" to "angle_util", "libEGL", "libGLESv2"
ninja -C out/Debug
export TRACE_LIBGL="/usr/lib/libGL.so.1" # may require a different path
apitrace trace -o mytrace ./out/Debug/hello_triangle
qapitrace mytrace
```

## Enabling General Logging

Normally, ANGLE only logs errors and warnings (e.g. to Android logcat).  General logging, or
additional levels of "trace" messages will be logged when the following GN arg is set:
```
angle_enable_trace = true
```

To log all GLES and EGL commands submitted by an application, including the following flag:
```
angle_enable_trace_events = true
```

If you want to enable `INFO`-level logs (and up) without incuring the log spam
of `angle_enable_trace`, you can instead use the following flag:
```
angle_always_log_info = true
```

## Debug Angle on Android

Android is built as an Android APK, which makes it more difficult to debug an APK that is using ANGLE.  The following information can allow you to debug ANGLE with LLDB.
* You need to build ANGLE with debug symbols enabled. Assume your build variant is called Debug. Make sure you have these lines in out/Debug/args.gn
```
is_component_build = false
is_debug = true
is_official_build = false
symbol_level = 2
strip_debug_info = false
ignore_elf32_limitations = true
angle_extract_native_libs = true
```
The following local patch may also be necessary:
```
diff --git a/build/config/compiler/compiler.gni b/build/config/compiler/compiler.gni
index 96a18d91a3f6..ca7971fdfd48 100644
--- a/build/config/compiler/compiler.gni
+++ b/build/config/compiler/compiler.gni
@@ -86,7 +86,8 @@ declare_args() {
   # Whether an error should be raised on attempts to make debug builds with
   # is_component_build=false. Very large debug symbols can have unwanted side
   # effects so this is enforced by default for chromium.
-  forbid_non_component_debug_builds = build_with_chromium
+  forbid_non_component_debug_builds = false 
```

Build/install/enable ANGLE apk for your application following other instructions.
* Modify gdbclient.py script to let it find the ANGLE symbols.
```
diff --git a/scripts/gdbclient.py b/scripts/gdbclient.py
index 61fac4000..1f43f4f64 100755
--- a/scripts/gdbclient.py
+++ b/scripts/gdbclient.py
@@ -395,6 +395,8 @@ def generate_setup_script(debugger_path, sysroot, linker_search_dir, binary_file
     vendor_paths = ["", "hw", "egl"]
     solib_search_path += [os.path.join(symbols_dir, x) for x in symbols_paths]
     solib_search_path += [os.path.join(vendor_dir, x) for x in vendor_paths]
+    solib_search_path += ["/your_path_to_chromium_src/out/Debug/lib.unstripped/"]
     if linker_search_dir is not None:
         solib_search_path += [linker_search_dir]
```
* Start your lldbclient.py from `/your_path_to_chromium_src/out/Debug` folder. This adds the ANGLE source-file paths to what is visible to LLDB, which allows LLDB to show ANGLE's source files. Refer to https://source.android.com/devices/tech/debug/gdb for how to attach the app for debugging.
* If you are debugging angle_perftests, you can use `--shard-timeout 100000000` to disable the timeout so that the test won't get killed while you are debugging. If the test runs too fast that you don't have time to attach, use `--delay-test-start=60` to give you extra time to attach.

## Forcing GL vendor and renderer strings

Some applications don't recognize ANGLE and lower their settings, refuse to start or even crash.
In those scenarios, you can force them to be values matching other devices.

On desktop:
```
ANGLE_GL_VENDOR="foo"
ANGLE_GL_RENDERER="bar"
ANGLE_GL_VERSION="blee"
```

On Android:
```
adb shell setprop debug.angle.gl_vendor "foo"
adb shell setprop debug.angle.gl_renderer "bar"
adb shell setprop debug.angle.gl_version "blee"
```

## Enabling Debug-Utils Markers

ANGLE can emit debug-utils markers for every GLES API command that are visible to both Android GPU
Inspector (AGI) and RenderDoc.  This support requires
[enabling general logging](#enabling-general-logging) as well as setting the following additional
GN arg:
```
angle_enable_annotator_run_time_checks = true
```
In addition, if the following GN arg is set, the API calls will output to Android's logcat:
```
angle_enable_trace_android_logcat = true
```
Once compiled, the markers need to be turned on.

### Turning on Debug Markers on Android

On Android, debug markers are turned on and off with an Android debug property that is
automatically deleted at the next reboot:

```
adb shell setprop debug.angle.markers 1
```

* 0: Turned off/disabled (default)
* 1: Turned on/enabled

### Turning on Debug Markers on Desktop

On desktop, debug markers are turned on and off with the ANGLE_ENABLE_DEBUG_MARKERS environment
variable (set in OS-specific manner):

* 0: Turned off/disabled (default)
* 1: Turned on/enabled

## Enable Vulkan Call Logging

ANGLE can output Vulkan API call information including detailed parameter info and state. Vulkan
call logging is available for ANGLE debug builds, builds with asserts enabled, or can be made
available on any build by setting the following GN arg:
```
angle_enable_vulkan_api_dump_layer = true
```

By default Vulkan call logging goes to stdout, or logcat on Android. Vulkan call logging can be
used with trace event and debug marker output as shown in [enabling general logging](#enabling-general-logging)
 and [enabling Debug-Utils markers](#enabling-debug-utils-markers)

### Vulkan Call Logging on Desktop

To log Vulkan calls on desktop set the environment variable `ANGLE_ENABLE_VULKAN_API_DUMP_LAYER` to 1.

For Vulkan call logging output to a file, set
the environment variable `VK_APIDUMP_LOG_FILENAME` to the correct location.

To show only Vulkan api calls without verbose parameter details set the environment variable
`VK_APIDUMP_DETAILED` to `false`

### Vulkan Call Logging on Android

Activate Vulkan call logging on Android by setting this Android debug property  that is
automatically deleted at the next reboot:
```
adb shell setprop debug.angle.enable_vulkan_api_dump_layer 1
```

For Vulkan call logging to a file on Android, specify the filename with an Android debug property that is
automatically deleted at the next reboot:
```
adb shell setprop debug.apidump.log_filename /data/data/[PACKAGE_NAME, i.e., com.android.angle.test for angle_trace_tests]/api_dump.txt
```

Similarly, detailed parameter output can be controlled by the following Android debug properties:
```
adb shell setprop debug.apidump.detailed false
```

## Running ANGLE under GAPID on Linux

[GAPID](https://github.com/google/gapid) can be used to capture trace of Vulkan commands on Linux.
When capturing traces of gtest based tests built inside Chromium checkout, make sure to run the
tests with `--single-process-tests` argument.

## Running ANGLE under GAPID on Android

[GAPID](https://github.com/google/gapid) can be used to capture a trace of the Vulkan or OpenGL ES
command stream on Android.  For it to work, ANGLE's libraries must have different names from the
system OpenGL libraries.  This is done with the gn arg:

```
angle_libs_suffix = "_ANGLE_DEV"
```

All
[AngleNativeTest](https://chromium.googlesource.com/chromium/src/+/main/third_party/angle/src/tests/test_utils/runner/android/java/src/com/android/angle/test/AngleNativeTest.java)
based tests share the same activity name, `com.android.angle.test.AngleUnitTestActivity`.
Thus, prior to capturing your test trace, the specific test APK must be installed on the device.
When you build the test, a test launcher is generated, for example,
`./out/Release/bin/run_angle_end2end_tests`. The best way to install the APK is to run this test
launcher once.

In GAPID's "Capture Trace" dialog, "Package / Action:" should be:

```
android.intent.action.MAIN:com.android.angle.test/com.android.angle.test.AngleUnitTestActivity
```

The mandatory [extra intent
argument](https://developer.android.com/studio/command-line/adb.html#IntentSpec) for starting the
activity is `org.chromium.native_test.NativeTest.StdoutFile`. Without it the test APK crashes. Test
filters can be specified via either the `org.chromium.native_test.NativeTest.CommandLineFlags` or
the `org.chromium.native_test.NativeTest.GtestFilter` argument.  Example "Intent Arguments:" values in
GAPID's "Capture Trace" dialog:

```
-e org.chromium.native_test.NativeTest.StdoutFile /sdcard/chromium_tests_root/out.txt -e org.chromium.native_test.NativeTest.CommandLineFlags "--gtest_filter=*ES2_VULKAN"
```

or

```
-e org.chromium.native_test.NativeTest.StdoutFile /sdcard/chromium_tests_root/out.txt --e org.chromium.native_test.NativeTest.GtestFilter RendererTest.SimpleOperation/ES2_VULKAN:SimpleOperationTest.DrawWithTexture/ES2_VULKAN
```

## Running ANGLE under RenderDoc

An application running through ANGLE can confuse [RenderDoc](https://github.com/baldurk/renderdoc),
as RenderDoc [hooks to EGL](https://github.com/baldurk/renderdoc/issues/1045) and ends up tracing
the calls the application makes, instead of the calls ANGLE makes to its backend.  As ANGLE is a
special case, there's little support for it by RenderDoc, though there are workarounds.

### Windows

On Windows, RenderDoc supports setting the environment variable `RENDERDOC_HOOK_EGL` to 0 to avoid
this issue.

### Linux

On Linux, there is no supported workaround by RenderDoc.  See [this
issue](https://github.com/baldurk/renderdoc/issues/1045#issuecomment-463999869).  To capture Vulkan
traces, the workaround is to build RenderDoc without GL(ES) support.

Building RenderDoc is straightforward.  However, here are a few instructions to keep in mind.

```
# Install dependencies based on RenderDoc document.  Here are some packages that are unlikely to be already installed:
$ sudo apt install libxcb-keysyms1-dev python3-dev qt5-qmake libqt5svg5-dev libqt5x11extras5-dev

# Inside the RenderDoc directory:
$ cmake -DCMAKE_BUILD_TYPE=Release -Bbuild -H. -DENABLE_GLES=OFF -DENABLE_GL=OFF

# QT_SELECT=5 is necessary if your distribution doesn't default to Qt5
$ QT_SELECT=5 make -j -C build

# Run RenderDoc from the build directory:
$ ./build/bin/qrenderdoc
```

If your distribution does not provide a recent Vulkan SDK package, you would need to manually
install that.  This script tries to perform this installation as safely as possible.  It would
overwrite the system package's files, so follow at your own risk.  Place this script just above the
extracted SDK directory.

```
#! /bin/bash

if [ $# -lt 1 ]; then
  echo "Usage: $0 <version>"
  exit 1
fi

ver=$1

if [ ! -d "$ver" ]; then
  echo "$ver is not a directory"
fi

# Verify everything first
echo "Verifying files..."
echo "$ver"/x86_64/bin/vulkaninfo
test -f "$ver"/x86_64/bin/vulkaninfo || exit 1
echo "$ver"/x86_64/etc/explicit_layer.d/
test -d "$ver"/x86_64/etc/explicit_layer.d || exit 1
echo "$ver"/x86_64/lib/
test -d "$ver"/x86_64/lib || exit 1

echo "Verified. Performing copy..."

echo sudo cp "$ver"/x86_64/bin/vulkaninfo /usr/bin/vulkaninfo
sudo cp "$ver"/x86_64/bin/vulkaninfo /usr/bin/vulkaninfo
echo sudo cp "$ver"/x86_64/etc/explicit_layer.d/* /etc/explicit_layer.d/
sudo cp "$ver"/x86_64/etc/explicit_layer.d/* /etc/explicit_layer.d/
echo sudo rm /usr/lib/x86_64-linux-gnu/libvulkan.so*
sudo rm /usr/lib/x86_64-linux-gnu/libvulkan.so*
echo sudo cp -P "$ver"/x86_64/lib/lib* /usr/lib/x86_64-linux-gnu/
sudo cp -P "$ver"/x86_64/lib/lib* /usr/lib/x86_64-linux-gnu/

echo "Done."
```

### Android
#### Using Linux as a Local Machine

If you are on Linux, make sure not to use the build done in the previous section.  The GL renderer
disabled in the previous section is actually needed in this section.

```
# Inside the RenderDoc directory:
# First delete the Cmake Cache in build/ directory
rm build/CMakeCache.txt

# Then build RenderDoc with cmake:
cmake -DCMAKE_BUILD_TYPE=Release -Bbuild -H.
QT_SELECT=5 make -j -C build
```

Follow
[Android Dependencies on Linux](https://github.com/baldurk/renderdoc/blob/v1.x/docs/CONTRIBUTING/Dependencies.md#android-dependencies-on-linux)
to download dependency files.

Define the following environment variables, for example in `.bashrc` (values are examples):

```
export JAVA_HOME=<path_to_jdk_root>
export ANDROID_SDK=<path_to_sdk_root>
export ANDROID_NDK=<path_to_ndk_root>
export ANDROID_NDK_HOME=<path_to_ndk_root>
```

In the renderdoc directory, create Android builds of RenderDoc:

```
mkdir build-android-arm32
cd build-android-arm32/
cmake -DBUILD_ANDROID=On -DANDROID_ABI=armeabi-v7a ..
make -j
cd ../

mkdir build-android-arm64
cd build-android-arm64/
cmake -DBUILD_ANDROID=On -DANDROID_ABI=arm64-v8a ..
make -j
cd ../
```

Note that you need both arm32 and arm64 builds even if working with an arm64 device.  See
[RenderDoc's documentation](https://github.com/baldurk/renderdoc/blob/v1.x/docs/CONTRIBUTING/Compiling.md#android)
for more information.

When you run RenderDoc, choose the "Replay Context" from the bottom-left part of the UI (defaults to
Local).  When selecting the device, you should see the RenderDoc application running.

In ANGLE itself, make sure you add a suffix for its names to be different from the system's.  Add
this to gn args:

```
angle_libs_suffix = "_ANGLE_DEV"
```

Next, you need to install an ANGLE test APK.  When you build the test, a test launcher is generated,
for example, `./out/Release/bin/run_angle_end2end_tests`. The best way to install the APK is to run
this test launcher once.

In RenderDoc, use `com.android.angle.test/com.android.angle.test.AngleUnitTestActivity` as the
Executable Path, and provide the following arguments:

```
-e org.chromium.native_test.NativeTest.StdoutFile /sdcard/chromium_tests_root/out.txt -e org.chromium.native_test.NativeTest.CommandLineFlags "--gtest_filter=*ES2_VULKAN"
```

Note that in the above, only a single command line argument is supported with RenderDoc.  If testing
dEQP on a non-default platform, the easiest way would be to modify `GetDefaultAPIName()` in
`src/tests/deqp_support/angle_deqp_gtest.cpp` (and avoid `--use-angle=X`).


#### Using Windows as a Local Machine
You should be able to download the latest [RenderDoc on Windows](https://renderdoc.org/builds) and follow the
[RenderDoc Official Documentation](https://renderdoc.org/docs/how/how_android_capture.html) for instructions on how to
use RenderDoc on Android. If you would like to build RenderDoc for Android on Windows yourself, you can follow the
[RenderDoc Officual Documentation](https://github.com/baldurk/renderdoc/blob/v1.x/docs/CONTRIBUTING/Compiling.md#android).
We listed more detailed instructions below on how to set up the build on Windows.

##### Install Android Dependencies

On windows, we need to install dependencies to build android, as described in
[RenderDoc Official Documentation](https://github.com/baldurk/renderdoc/blob/v1.x/docs/CONTRIBUTING/Dependencies.md#android)
1. Install [Android SDK](https://developer.android.com/about/versions/12/setup-sdk#install-sdk).

   Add a new system variable:

   Variable: ANDROID_SDK

   Value: path_to_sdk_directory (e.g. C:\Users\test\Appdata\Local\Android\Sdk)
2. Install [Android NDK](https://developer.android.com/studio/projects/install-ndk).

   Add a new system variable:

   Variable: ANDROID_NDK

   Value: path_to_ndk_directory (e.g. C:\Users\test\Appdata\Local\Android\Sdk\ndk\23.1.7779620)

3. Install [Java 8](https://www.oracle.com/java/technologies/downloads/#java8).

   Add a new system variable:

   Variable: JAVA_HOME

   Value: path_to_jdk1.8_directory (e.g. C:\Program Files\Java\jdk1.8.0_311)

5. Install [Android Debug Bridge](https://developer.android.com/studio/releases/platform-tools).

   Append android_sdk_platform-tools_directory to the Path system variable.

   e.g. C:\Users\Test\AppData\Local\Android\Sdk\platform-tools


##### Install Build Tools

1. Install a bash shell. Git Bash comes with Git installation on Windows should work.
2. Install [make](http://gnuwin32.sourceforge.net/packages/make.htm).
   Add the path to bin folder of GnuWin32 to the Path system variable.


##### Build RenderDoc Android APK on Windows

If you are using the Git Bash that comes with MinGW generator, you can run below commands to build Android APK
```
mkdir build-android-arm32
cd build-android-arm32/
cmake -DBUILD_ANDROID=On -DANDROID_ABI=armeabi-v7a -G "MinGW Makefiles" ..
make -j
cd ../

mkdir build-android-arm64
cd build-android-arm64/
cmake -DBUILD_ANDROID=On -DANDROID_ABI=arm64-v8a -G "MinGW Makefiles" ..
make -j
cd ../
```
If the generator type of the bash shell you are using is different from MinGW, replace the "MinGW" in the above cmake
command with the generator
type you are using, as described in
[RenderDoc Official Documentation](https://github.com/baldurk/renderdoc/blob/v1.x/docs/CONTRIBUTING/Compiling.md#android).


##### Build Errors And Resolutions

* **cmake command errors**

```
Error: Failed to run MSBuild command:
C:/Program Files (x86)/Microsoft Visual Studio/2019/Professional/MSBuild/Current/Bin/MSBuild.exe to get the value of
VCTargetsPath:
error : The BaseOutputPath/OutputPath property is not set for project 'VCTargetsPath.vcxproj'.
Please check to make sure that you have specified a valid combination of Configuration and Platform for this project.
Configuration='Debug'  Platform='x64'.
```

This is due to the cmake command is using Visual Studio as the generator type. Run the cmake command with the
generator type "MinGW Makefiles" or "MSYS Makefiles".

```Error: Does not match the generator used previously```



Delete the CMakeCache file in build directories build-android-arm64/ or build-android-arm32/.


* **make command errors**

```
-Djava.ext.dirs is not supported.
Error: Could not create the Java Virtual Machine.
Error: A fatal exception has occurred. Program will exit.

```

Downgrade Java JDK version to [Java 8](https://www.oracle.com/java/technologies/downloads/#java8).


##### Steps to use the RenderDoc you just built
1. Build arm32 and arm64 android packages. See [instructions](#build-renderdoc-android-apk-on-windows) in the above
section.

2. Uninstall the renderdoc package.

This step is required if you have installed / used RenderDoc on the same Android device before. RenderDoc only pushes
the renderdoccmd APK to the Android device if it finds the version of the existing APK on the device is different from
the version of the APK we are going to install, and the version is dictated by the git hash it was built from. Therefore
any local modifications in the RenderDoc codebase would not get picked up if we don't uninstall the old APK first.

```
adb uninstall org.renderdoc.renderdoccmd.arm64
adb uninstall org.renderdoc.renderdoccmd.arm32
```
3. Build renderdoc on windows desktop by clicking "build solution" in visual studio.
4. Launch renderdoc from visual studio, and push the android packages to android device by selecting the connected
device at the bottom left corner.

### Add SPIRV-to-GLSL Shader View Option
RenderDoc allows us to add and configure customized shader processing tools:
https://renderdoc.org/docs/window/settings_window.html#shader-processing-tools-config.

To configure RenderDoc to display shader source code in GLSL, instead of spirv,
follow the below steps:


1. Get the SPIRV-Cross tool:

Clone the SPIRV-Cross git repo: https://github.com/KhronosGroup/SPIRV-Cross:
```
git clone https://github.com/KhronosGroup/SPIRV-Cross.git
```
Compile the SPIRV-Cross:
```
# inside SPIRV-Cross directory
make
```
2. Open Shader Viewer Settings window: RenderDoc -> Tools -> Settings, and select
   Shader Viewer on the left.
3. Click Add on the bottom to add a new tool, and fill the new tool details:

| Item       | Value                               |
|------------|-------------------------------------|
| Name       | SPIRV-CROSS                         |
| Tool Type  | SPIRV-Cross                         |
| Executable | <spirv-cross-repo-root>/spirv-cross |

5. Restart RenderDoc.

## Testing with Chrome Canary

Many of ANGLE's OpenGL ES entry points are exposed in Chromium as WebGL 1.0 and WebGL 2.0 APIs that
are available via JavaScript. For testing purposes, custom ANGLE builds may be injected in Chrome
Canary.

### Setup

#### Windows

1. Download and install [Google Chrome Canary](https://www.google.com/chrome/canary/).
2. Build ANGLE x64, Release.
3. Run `python scripts\update_chrome_angle.py` to replace Canary's ANGLE with your custom ANGLE
   (note: Canary must be closed).

#### Linux

1. Install Google Chrome Dev (via apt, or otherwise).  Expected installation directory is
   `/opt/google/chrome-unstable`.
2. Build ANGLE for the running platform.  `is_component_build = false` is suggested in the GN args.
3. Run `python scripts/update_chrome_angle.py` to replace Dev's ANGLE with your custom ANGLE
4. Add ANGLE's build path to the `LD_LIBRARY_PATH` environment variable.

#### macOS

1. Download and install [Google Chrome Canary](https://www.google.com/chrome/canary/).
2. Build ANGLE for the running platform; GN args should contain `is_debug = false`.
3. Run `./scripts/update_chrome_angle.py` to replace Canary's ANGLE with your custom ANGLE.

### Usage

Run Chrome:

- On Windows: `%LOCALAPPDATA%\Google\Chrome SxS\chrome.exe`
- On Linux: `/opt/google/chrome-unstable/google-chrome-unstable`
- On macOS: `./Google\ Chrome\ Canary.app/Contents/MacOS/Google\ Chrome\ Canary`

With the following command-line options:

* `--use-cmd-decoder=passthrough --use-gl=angle` and one of
  * `--use-angle=d3d9` (Direct3D 9 renderer, Windows only)
  * `--use-angle=d3d11` (Direct3D 11 renderer, Windows only)
  * `--use-angle=d3d11on12` (Direct3D 11on12 renderer, Windows only)
  * `--use-angle=gl` (OpenGL renderer)
  * `--use-angle=gles` (OpenGL ES renderer)
  * `--use-angle=vulkan` (Vulkan renderer)
  * `--use-angle=swiftshader` (SwiftShader renderer)
  * `--use-angle=metal` (Metal renderer, macOS only)

Additional useful options:

* `--enable-logging`: To see logs
* `--disable-gpu-watchdog`: To disable Chromium's watchdog, killing the GPU process when slow (due
  to a debug build for example)
* `--disable-gpu-sandbox`: To disable Chromium's sandboxing features, if it's getting in the way of
  testing.
* `--disable-gpu-compositing`: To make sure only the WebGL test being debugged is run through ANGLE,
  not the entirety of Chromium.
