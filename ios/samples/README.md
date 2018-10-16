# Filament sample iOS app

This directory contains a sample app that demonstrates how to use the Filament API in an iOS
application.

## Prequisites

iOS support for Filament is experimental. Currently, both the OpenGL ES 3.0 and Vulkan (via
MoltenVK) backends are supported. The iOS simulator is not supported yet.

Before attempting to build for iOS, read [Filament's README](../../README.md). You must first
cross-compile Filament for ARM64 on a macOS host before running the sample. The easiest way to do
this is by running the following command at the root of Filament's source tree:

```
$ ./build.sh -p ios debug
```

This will automatically download Apple's iOS toolchain file for CMake and build Filament for iOS.

Afterwards, you should find all the Filament libraries inside of `out/ios-debug/filament/lib/arm64`:

```
libbluevk.a
libfilabridge.a
libfilaflat.a
libfilament.a
libutils.a
```

The sample application expects the libraries to be present here. Likewise, when building the sample
in Release mode, the linker looks for the libraries in `out/ios-release/filament/lib/arm64`. To
build Filament in Release mode, replace `debug` with `release` in the above `build.sh` command.

If using the Vulkan backend, the pre-compiled `libMoltenVK.dylib` library must be present in
`third_party/moltenvk/ios`. When building, the Xcode project automatically signs and includes this
library inside the application bundle.

## Xcode

Open up the Xcode project at `ios/samples/hello-triangle/hello-triangle.xcodeproj`. The
hello-triangle project contains two schemes, `hello-triangle-VK` and `hello-triangle-OpenGL`, which
use the Vulkan and OpenGL backends respectively. Before building you will need to select one of the
schemes, sign in to your Apple developer account, and select an appropriate development team in the
project editor.

## Building

At this point you should be able to hit the "Run" icon to run the sample on your device.
