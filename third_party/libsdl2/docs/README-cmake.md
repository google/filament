CMake
================================================================================
(www.cmake.org)

SDL's build system was traditionally based on autotools. Over time, this
approach has suffered from several issues across the different supported 
platforms.
To solve these problems, a new build system based on CMake is under development.
It works in parallel to the legacy system, so users can experiment with it
without complication.
While still experimental, the build system should be usable on the following
platforms:

* FreeBSD
* Linux
* VS.NET 2010
* MinGW and Msys
* macOS, iOS, and tvOS, with support for XCode


================================================================================
Usage
================================================================================

Assuming the source for SDL is located at ~/sdl

    cd ~
    mkdir build
    cd build
    cmake ../sdl

This will build the static and dynamic versions of SDL in the ~/build directory.


================================================================================
Usage, iOS/tvOS
================================================================================

CMake 3.14+ natively includes support for iOS and tvOS.  SDL binaries may be built
using Xcode or Make, possibly among other build-systems.

When using a recent version of CMake (3.14+), it should be possible to:

- build SDL for iOS, both static and dynamic
- build SDL test apps (as iOS/tvOS .app bundles)
- generate a working SDL_config.h for iOS (using SDL_config.h.cmake as a basis)

To use, set the following CMake variables when running CMake's configuration stage:

- `CMAKE_SYSTEM_NAME=<OS>`   (either `iOS` or `tvOS`)
- `CMAKE_OSX_SYSROOT=<SDK>`  (examples: `iphoneos`, `iphonesimulator`, `iphoneos12.4`, `/full/path/to/iPhoneOS.sdk`,
                              `appletvos`, `appletvsimulator`, `appletvos12.4`, `/full/path/to/AppleTVOS.sdk`, etc.)
- `CMAKE_OSX_ARCHITECTURES=<semicolon-separated list of CPU architectures>` (example: "arm64;armv7s;x86_64")


### Examples (for iOS/tvOS):

- for iOS-Simulator, using the latest, installed SDK:

    `cmake ~/sdl -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_SYSROOT=iphonesimulator -DCMAKE_OSX_ARCHITECTURES=x86_64`

- for iOS-Device, using the latest, installed SDK, 64-bit only

    `cmake ~/sdl -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_SYSROOT=iphoneos -DCMAKE_OSX_ARCHITECTURES=arm64`

- for iOS-Device, using the latest, installed SDK, mixed 32/64 bit

    `cmake ~/sdl -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_SYSROOT=iphoneos -DCMAKE_OSX_ARCHITECTURES="arm64;armv7s"`

- for iOS-Device, using a specific SDK revision (iOS 12.4, in this example):

    `cmake ~/sdl -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_SYSROOT=iphoneos12.4 -DCMAKE_OSX_ARCHITECTURES=arm64`

- for iOS-Simulator, using the latest, installed SDK, and building SDL test apps (as .app bundles):

    `cmake ~/sdl -DSDL_TEST=1 -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_SYSROOT=iphonesimulator -DCMAKE_OSX_ARCHITECTURES=x86_64`

- for tvOS-Simulator, using the latest, installed SDK:

    `cmake ~/sdl -DCMAKE_SYSTEM_NAME=tvOS -DCMAKE_OSX_SYSROOT=appletvsimulator -DCMAKE_OSX_ARCHITECTURES=x86_64`

- for tvOS-Device, using the latest, installed SDK:

    `cmake ~/sdl -DCMAKE_SYSTEM_NAME=tvOS -DCMAKE_OSX_SYSROOT=appletvos -DCMAKE_OSX_ARCHITECTURES=arm64`
