# Filament sample iOS apps

This directory contains sample apps that demonstrate how to use the Filament API in an iOS
application.

## Prerequisites

Currently, both the OpenGL ES 3.0 and Metal backends are supported, but using the Metal backend is
recommended.

Filament is kept up-to-date with Apple's latest SDK and thus must be built using the latest Xcode.
Pre-built binaries can be accessed from [Filament
releases](https://github.com/google/filament/releases).

The iOS Metal backend is supported on iOS 11 and up. The OpenGL ES backend should be supported on iOS 7
and up, though it is not regularly tested.

The iOS simulator is also supported for both the OpenGL and Metal backends, but please be aware that
some rendering features (such as shadows) might not work properly in the simulator.

Before attempting to build for iOS, read [Filament's README](../../README.md). You must first
cross-compile Filament for desktop and ARM64 on a macOS host before running the sample. The easiest
way to do this is by running the following commands at the root of Filament's source tree:

```
$ ./build.sh -p desktop -i release         # build required desktop tools (matc, resgen, etc)
$ ./build.sh -p ios -i debug               # cross-compile Filament for iOS
```

This will automatically download Apple's iOS toolchain file for CMake and build Filament for iOS.

Afterwards, you should find all the Filament libraries inside of `out/ios-debug/filament/lib/arm64`:

```
libfilabridge.a
libfilaflat.a
libfilament.a
libbackend.a
libutils.a
libsmol-v.a
libgeometry.a
```

The sample applications expect the libraries to be present here. Likewise, when building the sample
in Release mode, the linker looks for the libraries in `out/ios-release/filament/lib/arm64`. To
build Filament in Release mode, replace `debug` with `release` in the above `build.sh` command.

If you also want to be able to run on the iOS simulator, add the `-s` flag to the `build.sh`
command. For example, the following command will build for both devices (ARM64) and the simulator
(x86_64) in Debug mode:

```
$ ./build.sh -s -p ios -i debug
```

When building for the simulator, the sample will then link against the libraries present in
`out/ios-debug/filament/lib/x86_64`.

## Xcode

Open up one of the Xcode projects:

- gltf-viewer/gltf-viewer.xcodeproj
- hello-ar/hello-ar.xcodeproj
- hello-gltf/hello-gltf.xcodeproj
- hello-pbr/hello-pbr.xcodeproj
- hello-triangle/hello-triangle.xcodeproj
- transparent-rendering/transparent-rendering.xcodeproj

Each project contains two schemes, `<sample> Metal` and `<sample> OpenGL`, which use the Metal and
OpenGL backends respectively. Before building you will need to select one of the schemes, sign in to
your Apple developer account, select an appropriate development team in the project editor, and
change the bundle identifier to a unique name.

## Building

At this point you should be able to hit the "Run" icon to run the sample on your device.
Alternatively, if you have also built Filament for the simulator, select an iPhone simulator.

## Build errors

Occasionally, changes in Filament's `CMakeLists.txt` can require cleaning the CMake build
directories. This can be done by adding the `-c` flag to the `build.sh` script or by simply deleting
the `out` directory.

Before reporting build errors, please try cleaning the CMake directories first and building again
from scratch.

## XcodeGen

[XcodeGen](https://github.com/yonaskolb/XcodeGen) version 2.43.0 is used to generate the Xcode
projects. While not required to run the samples, XcodeGen makes modifying them easier. Each sample
folder contains the `project.yml` file used for the sample, which includes a global
`app-template.yml` file. Simply run

```
$ xcodegen
```

within a sample folder to re-generate the Xcode project. You may need to close and re-open the
project in Xcode to see changes take effect.

## Building iOS Samples with ASan / UBSan

1. Turn on ASan / UBSan in Filament's top-level CMakeLists.txt by passing
   `-DFILAMENT_ENABLE_ASAN_UBSAN=1` to trigger the following line:

```
set(EXTRA_SANITIZE_OPTIONS "-fsanitize=undefined -fsanitize=address")
```

2. In the Xcode project, navigate to Product -> Scheme -> Edit Scheme... Under the Diagnostics tab,
   check the box next for Address Sanitizer and Undefined Behavior.

3. Build Filament and run the iOS sample as usual. Any errors will cause an exception to raise and
   diagnostics to print in the console.
