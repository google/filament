# Filament sample iOS apps

This directory contains sample apps that demonstrate how to use the Filament API in an iOS
application.

## Prequisites

iOS support for Filament is experimental. Currently, both the OpenGL ES 3.0 and Metal backends are
supported. Building for the iOS simulator is also supported, but only for the OpenGL backend
(Apple's simulator has no support for Metal).

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
libutils.a
libsmol-v.a
```

The sample applications expect the libraries to be present here. Likewise, when building the sample
in Release mode, the linker looks for the libraries in `out/ios-release/filament/lib/arm64`. To
build Filament in Release mode, replace `debug` with `release` in the above `build.sh` command.

If you also want to be able to run on the iOS simulator, add the `-s` flag to the `build.sh`
command. For example, the following command will build for both devices (ARM64) and the simulator
(x86_64) in Debug mode:

```
$ ./build.sh -s -p ios debug
```

When building for the simulator, the sample will then link against the libraries present in
`out/ios-debug/filament/lib/x86_64`.

## Xcode

Open up one of the Xcode projects:
    - hello-triangle/hello-triangle.xcodeproj
    - hello-pbr/hello-pbr.xcodeproj

Each project contains two schemes, `*-Metal` and `*-OpenGL`, which use the Metal and OpenGL backends
respectively. Before building you will need to select one of the schemes, sign in to your Apple
developer account, and select an appropriate development team in the project editor.

## Building

At this point you should be able to hit the "Run" icon to run the sample on your device.
Alternatively, if you have also built Filament for the simulator, select an iPhone simulator.

## Build errors

Occasionally, changes in Filament's `CMakeLists.txt` can require cleaning the CMake build
directories. This can be done by adding the `-c` flag to the `build.sh` script or by simply deleting
the `out` directory.

Before reporting build errors, please try cleaning the CMake directories first and building again
from scratch.
