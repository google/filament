# ANGLE for Android

**Important note**: Android builds currently require Linux.

## Setting up the ANGLE build for Android

Please follow the instructions in [DevSetup](DevSetup.md) to check out and bootstrap ANGLE with
gclient. Then edit your `.gclient` to add `target_os = ['android']` to check out Android
dependencies. Then run `gclient sync` to download all required sources and packages.

The following command will open a text editor to populate GN args for an Android Release build:
```
gn args out/Android
```

Once the editor is up, paste the following GN args to generate an Android build, and save the file.
```
target_os = "android"
target_cpu = "arm64"
is_component_build = false
is_debug = false
angle_assert_always_on = true   # Recommended for debugging. Turn off for performance.
use_remoteexec = true           # Googlers-only! If you're not a Googler remove this.
```

More targeted GN arg combinations can be found [below](#android-gn-args-combinations).

If you run into any problems with the above, you can copy the canonical args from CI:
 - Visit the ANGLE [CI Waterfall](https://ci.chromium.org/p/angle/g/ci/console).
 - Open any recent Android build.
 - Expand the for "lookup GN args" step and copy the GN args.
 - If you are not a Googler, also omit the `use_remoteexec` flag.

## Building ANGLE for Android

Build all ANGLE targets using the following command:

```
autoninja -C out/Android
```

Most ANGLE build targets are supported. We do not support the ANGLE samples on
Android currently. ANGLE tests will be in your `out/Android` directory, and can
be run with various options. For instance, angle perftests can be run with:

```
./out/Android/angle_perftests --verbose --local-output --gtest_filter=DrawCallPerf*
```

Additional details are in [Android Test Instructions][AndroidTest].

Additional Android dEQP notes can be found in [Running dEQP on Android](dEQP.md#Running-dEQP-on-Android).

If you are targeting WebGL and want to run with ANGLE, you will need to build within a full
Chromium checkout. Please follow the [Chromium build instructions for Android][ChromeAndroid].
Also refer to the [ANGLE Guide][ANGLEChrome] on how to work with Top of Tree ANGLE in Chromium.
Build the `chrome_public_apk` target, and follow the [GPU Testing][GPU Testing] doc, using
`--browser=android-chromium`. Make sure to set your `CHROMIUM_OUT_DIR` environment variable, so
that your browser is found, otherwise the tests will use the stock browser.

[AndroidTest]: https://chromium.googlesource.com/chromium/src/+/main/docs/testing/android_test_instructions.md
[GPU Testing]: http://www.chromium.org/developers/testing/gpu-testing#TOC-Running-the-GPU-Tests-Locally
[ChromeAndroid]: https://chromium.googlesource.com/chromium/src/+/main/docs/android_build_instructions.md
[ANGLEChrome]: BuildingAngleForChromiumDevelopment.md

## Using ANGLE as the Android OpenGL ES driver

Starting with Android 10 (Q), you can load ANGLE as your device's OpenGL ES driver.

`== Important Note ==` You can only run this ANGLE with *DEBUGGABLE APPS* or when you have
*ROOT ACCESS*. Debuggable apps are [marked debuggable][Debuggable] in the manifest. For root
access, see the [Android documentation][UserDebug] for how to build from source.

To build the ANGLE APK, you must first bootstrap your build by following the steps
[above](#ANGLE-for-Android). The steps below will result in an APK that contains the ANGLE
libraries and can be installed on any Android 10+ build.

Apps can be opted in to ANGLE [one at a time](#ANGLE-for-a-single-OpenGL-ES-app), in
[groups](#ANGLE-for-multiple-OpenGL-ES-apps), or [globally](#ANGLE-for-all-OpenGL-ES-apps). The
apps must be launched by the Java runtime since the libraries are discovered within an installed
package. This means ANGLE cannot be used by native executables or SurfaceFlinger at this time.

## Building the ANGLE APK

Using `gn args` from above, you can build the ANGLE apk using:
```
autoninja -C out/Android angle_apks
```

## Installing the ANGLE APK

```
adb install -r -d --force-queryable out/Android/apks/AngleLibraries.apk
```
You can verify installation by looking for the package name:
```
$ adb shell pm path org.chromium.angle
package:/data/app/org.chromium.angle-HpkUceNFjoLYKPbIVxFWLQ==/base.apk
```

Note that `angle_debug_package` must be set to `org.chromium.angle` for this apk to be loaded.

## Selecting ANGLE as the OpenGL ES driver

For debuggable applications or root users, you can tell the platform to load ANGLE libraries from
the installed package.
```
adb shell settings put global angle_debug_package org.chromium.angle
```
Remember that ANGLE can only be used by applications launched by the Java runtime.

Note: Side-loading apk on Cuttlefish currently requires [special setup](#Cuttlefish-setup)

## ANGLE driver choices

There are multiple values you can use for selecting which OpenGL ES driver is loaded by the platform.

The following values are supported for `angle_gl_driver_selection_values`:
 - `angle` : Use ANGLE.
 - `native` : Use the native OpenGL ES driver.
 - `default` : Use the default driver. This allows the platform to decide which driver to use.

In each section below, replace `<driver>` with one of the values above.

### ANGLE for a *single* OpenGL ES app

```
adb shell settings put global angle_gl_driver_selection_pkgs <package name>
adb shell settings put global angle_gl_driver_selection_values <driver>
```

### ANGLE for *multiple* OpenGL ES apps

Similar to selecting a single app, you can select multiple applications by listing their package
names and driver choice in comma separated lists.  Note the lists must be the same length, one
driver choice per package name.
```
adb shell settings put global angle_gl_driver_selection_pkgs <package name 1>,<package name 2>,<package name 3>,...
adb shell settings put global angle_gl_driver_selection_values <driver 1>,<driver 2>,<driver 3>,...
```

### ANGLE for *all* OpenGL ES apps

`Note: This method only works on a device with root access.`

Enable:
```
adb shell settings put global angle_gl_driver_all_angle 1
```
Disable:
```
adb shell settings put global angle_gl_driver_all_angle 0
```

## Check for success

Check to see that ANGLE was loaded by your application:
```
$ adb logcat -d | grep ANGLE
V GraphicsEnvironment: ANGLE developer option for <package name>: angle
I GraphicsEnvironment: ANGLE package enabled: org.chromium.angle
I ANGLE   : Version (2.1.0.f87fac56d22f), Renderer (Vulkan 1.1.87(Adreno (TM) 615 (0x06010501)))
```

Note that this might be logged by the built-in ANGLE and not the installed apk if `angle_debug_package` wasn't set.

## Clean up

Settings persist across reboots, so it is a good idea to delete them when finished.
```
adb shell settings delete global angle_debug_package
adb shell settings delete global angle_gl_driver_all_angle
adb shell settings delete global angle_gl_driver_selection_pkgs
adb shell settings delete global angle_gl_driver_selection_values
```

## Troubleshooting

If your application is not debuggable or you are not root, you may see an error like this in the log:
```
$ adb logcat -d | grep ANGLE
V GraphicsEnvironment: ANGLE developer option for <package name>: angle
E GraphicsEnvironment: Invalid number of ANGLE packages. Required: 1, Found: 0
E GraphicsEnvironment: Failed to find ANGLE package.
```
Double check that you are root, or that your application is [marked debuggable][Debuggable].

## Android GN args combinations

The [above](#angle-gn-args-for-android) GN args only modify default values to generate a Debug
build for Android. Below are some common configurations used for different scenarios.

To determine what is different from default, you can point the following command at your target
directory. It will show the list of gn args in use, where they came from, their current value,
and their default values.
```
gn args --list <dir>
```

### Performance config

This config is designed to get maximum performance by disabling debug configs and validation layers.
Note: The oddly named `is_official_build` is a more aggressive optimization level than `Release`. Its name is historical.
```
target_os = "android"
target_cpu = "arm64"
angle_enable_vulkan = true
is_component_build = false
is_official_build = true
is_debug = false
```

### Debug config

This config is useful for quickly ensuring Vulkan is running cleanly. It disables debug, but
enables asserts and allows validation errors.
```
target_os = "android"
target_cpu = "arm64"
is_component_build = false
is_debug = true
```

#### Application Compatibility

Application compatibility may be increased by enabling non-conformant features and extensions with
a GN arg:

```
angle_expose_non_conformant_extensions_and_versions = true
```

### Cuttlefish setup

Cuttlefish uses ANGLE as a system GL driver, on top of SwiftShader. It also uses SkiaGL (not SkiaVk)
due to a SwiftShader limitation. This enables preloading of GL libs - so in this case, ANGLE - into Zygote,
which with the current implementation of the loader results in system libs being loaded instead of
loading them from the debug apk. To workaround, a custom library name can be set via a GN arg:

```
angle_libs_suffix = _angle_in_apk
```

and enabled in the platform with this setting (mind the lack of a leading underscore compared to the above):

```
adb shell setprop debug.angle.libs.suffix angle_in_apk
```

## Accessing ANGLE traces

To sync and build the ANGLE traces, jump to [ANGLE Restricted Traces](https://chromium.googlesource.com/angle/angle.git/+/HEAD/src/tests/restricted_traces/README.md#angle-restricted-traces).

## Command line for launching chrome on Android

[This Makefile](https://github.com/phuang/test/blob/main/chromium/Makefile) contains many useful
command lines for launching chrome.

Targets run_chrome_public_apk_* is for launching chrome on Android.

To use this Makefile, download it into chrome build tree, and use below commands (for more targets please check Makefile)
```
# To edit gn args
$ make args OUT=out_android/Release  # The OUT can be set in Makefile instead of passing it in command line

# Build and run chrome on Android device with GLRenderer
$ make run_chrome_public_apk_gl

# Build and run chrome on Android device with SkiaRenderer
$ make run_chrome_public_apk_skia

# Run adb logcat
$ make adb_logcat

# Symbolize Android crash stack
$ make android_symbol

# Build and run gpu_unittests
$ make gpu_unittests GTEST_FILTER="gtest-filters" # If GTEST_FILTER is not specified, all tests will be run.
```

[Debuggable]: https://developer.android.com/guide/topics/manifest/application-element#debug
[UserDebug]: https://source.android.com/setup/build/building
