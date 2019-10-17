# Filament

<img alt="Android" src="build/img/android.png" width="20px" height="20px" hspace="2px"/>[![Android Build Status](https://filament-build.storage.googleapis.com/badges/build_status_android.svg)](https://filament-build.storage.googleapis.com/badges/build_link_android.html)
<img alt="iOS" src="build/img/macos.png" width="20px" height="20px" hspace="2px"/>[![iOS Build Status](https://filament-build.storage.googleapis.com/badges/build_status_ios.svg)](https://filament-build.storage.googleapis.com/badges/build_link_ios.html)
<img alt="Linux" src="build/img/linux.png" width="20px" height="20px" hspace="2px"/>[![Linux Build Status](https://filament-build.storage.googleapis.com/badges/build_status_linux.svg)](https://filament-build.storage.googleapis.com/badges/build_link_linux.html)
<img alt="macOS" src="build/img/macos.png" width="20px" height="20px" hspace="2px"/>[![MacOS Build Status](https://filament-build.storage.googleapis.com/badges/build_status_mac.svg)](https://filament-build.storage.googleapis.com/badges/build_link_mac.html)
<img alt="Windows" src="build/img/windows.png" width="20px" height="20px" hspace="2px"/>[![Windows Build Status](https://filament-build.storage.googleapis.com/badges/build_status_windows.svg)](https://filament-build.storage.googleapis.com/badges/build_link_windows.html)
<img alt="Web" src="build/img/web.png" width="20px" height="20px" hspace="2px"/>[![Web Build Status](https://filament-build.storage.googleapis.com/badges/build_status_web.svg)](https://filament-build.storage.googleapis.com/badges/build_link_web.html)

Filament is a real-time physically based rendering engine for Android, iOS, Linux, macOS, Windows,
and WebGL. It is designed to be as small as possible and as efficient as possible on Android.

Filament is currently used in the
[Sceneform](https://developers.google.com/ar/develop/java/sceneform/) library both at runtime on
Android devices and as the renderer inside the Android Studio plugin.

## Download

[Download Filament releases](https://github.com/google/filament/releases) to access stable builds.

Make sure you always use tools from the same release as the runtime library. This is particularly
important for `matc` (material compiler).

If you prefer to live on the edge, you can download a continuous build by clicking one of the build
badges above.

## Documentation

- [Filament](https://google.github.io/filament/Filament.html), an in-depth explanation of
  real-time physically based rendering, the graphics capabilities and implementation of Filament.
  This document explains the math and reasoning behind most of our decisions. This document is a
  good introduction to PBR for graphics programmers.
- [Materials](https://google.github.io/filament/Materials.html), the full reference
  documentation for our material system. This document explains our different material models, how
  to use the material compiler `matc` and how to write custom materials.
- [Material Properties](https://google.github.io/filament/Material%20Properties.pdf), a reference
  sheet for the standard material model.

## Samples

Here are a few sample materials rendered with Filament:

![Damaged Helmet](docs/images/samples/model_damaged_helmet.jpg)
![Helmet](docs/images/samples/model_helmet.jpg)
![Brushed copper](docs/images/samples/brushed_copper_2.jpg)
![Material 1](docs/images/samples/material_01.jpg)
![Material 2](docs/images/samples/material_02.jpg)
![Material 6](docs/images/samples/material_06.jpg)
![Material 8](docs/images/samples/material_08.jpg)

## Applications

Here are a few screenshots of applications that use Filament in production:

### Google Maps AR Navigation

![Google Maps AR Navigation](docs/images/samples/app_gmm_ar_nav.jpg)

### Google Search 3D/AR Viewer on Android

![Google Search 3D/AR Viewer on Android](docs/images/samples/app_google_3d_viewer.jpg)

## Features

### APIs

- Native C++ API for Android, iOS, Linux, macOS and Windows
- Java/JNI API for Android, Linux, macOS and Windows
- JavaScript API

### Backends

- OpenGL 4.1+ for Linux, macOS and Windows
- OpenGL ES 3.0+ for Android and iOS
- Metal for macOS and iOS
- Vulkan 1.0 for Android, Linux, macOS and iOS (with MoltenVk), and Windows
- WebGL 2.0 for all platforms

### Rendering

- Clustered forward renderer
- Cook-Torrance microfacet specular BRDF
- Lambertian diffuse BRDF
- HDR/linear lighting
- Metallic workflow
- Clear coat
- Anisotropic lighting
- Approximated translucent (subsurface) materials
- Cloth shading
- Normal mapping & ambient occlusion mapping
- Image-based lighting
- Physically-based camera (shutter speed, sensitivity and aperture)
- Physical light units
- Point light, spot light and directional light
- SSAO
- ACES-like tone-mapping
- Temporal dithering
- FXAA, MSAA and specular anti-aliasing
- Dynamic resolution (on Android and iOS)

### Future

Many other features have been either prototyped or planned:

- IES light profiles/cookies
- Area lights
- Fog
- Color grading
- Bloom
- TAA
- etc.

## Directory structure

This repository not only contains the core Filament engine, but also its supporting libraries
and tools.

- `android`:               Android libraries and projects
  - `build`:               Custom Gradle tasks for Android builds
  - `filamat-android`:     Filament material generation library (AAR) for Android
  - `filament-android`:    Filament library (AAR) for Android
  - `samples`:             Android-specific Filament samples
- `art`:                   Source for various artworks (logos, PDF manuals, etc.)
- `assets`:                3D assets to use with sample applications
- `build`:                 CMake build scripts
- `docs`:                  Documentation
  - `math`:                Mathematica notebooks used to explore BRDFs, equations, etc.
- `filament`:              Filament rendering engine (minimal dependencies)
- `ide`:                   Configuration files for IDEs (CLion, etc.)
- `ios`:                   Sample projects for iOS
- `java`:                  Java bindings for Filament libraries
- `libs`:                  Libraries
  - `bluegl`:              OpenGL bindings for macOS, Linux and Windows
  - `bluevk`:              Vulkan bindings for macOS, Linux, Windows and Android
  - `filabridge`:          Library shared by the Filament engine and host tools
  - `filaflat`:            Serialization/deserialization library used for materials
  - `filagui`:             Helper library for [Dear ImGui](https://github.com/ocornut/imgui)
  - `filamat`:             Material generation library
  - `filameshio`:          Tiny filamesh parsing library (see also `tools/filamesh`)
  - `geometry`:            Mesh-related utilities
  - `gltfio`:              Loader and optional pipeline for glTF 2.0
  - `ibl`:                 IBL generation tools
  - `image`:               Image filtering and simple transforms
  - `imageio`:             Image file reading / writing, only intended for internal use
  - `math`:                Math library
  - `rays`:                Simple path tracer used for baking ambient occlusion, etc.
  - `utils`:               Utility library (threads, memory, data structures, etc.)
- `samples`:               Sample desktop applications
- `shaders`:               Shaders used by `filamat` and `matc`
- `third_party`:           External libraries and assets
  - `environments`:        Environment maps under CC0 license that can be used with `cmgen`
  - `models`:              Models under permissive licenses
  - `textures`:            Textures under CC0 license
- `tools`:                 Host tools
  - `cmgen`:               Image-based lighting asset generator
  - `filamesh`:            Mesh converter
  - `glslminifier`:        Minifies GLSL source code
  - `matc`:                Material compiler
  - `matinfo`              Displays information about materials compiled with `matc`
  - `mipgen`               Generates a series of miplevels from a source image
  - `normal-blending`:     Tool to blend normal maps
  - `resgen`               Aggregates binary blobs into embeddable resources
  - `roughness-prefilter`: Pre-filters a roughness map from a normal map to reduce aliasing
  - `skygen`:              Physically-based sky environment texture generator
  - `specular-color`:      Computes the specular color of conductors based on spectral data
- `web`:                   JavaScript bindings, documentation, and samples

## Building Filament

### Prerequisites

To build Filament, you must first install the following tools:

- CMake 3.10 (or more recent)
- clang 7.0 (or more recent)
- [ninja 1.8](https://github.com/ninja-build/ninja/wiki/Pre-built-Ninja-packages) (or more recent)

To build the Java based components of the project you can optionally install (recommended):

- OpenJDK 1.8 (or more recent)

Additional dependencies may be required for your operating system. Please refer to the appropriate
section below.

Building the `rays` library (used for light baking) is optional and requires the following packages:

- embree 3.0+
- libtbb-dev

To build Filament for Android you must also install the following:

- Android Studio 3.3
- Android SDK
- Android NDK 19 or higher

### Environment variables

Make sure the environment variable `ANDROID_HOME` points to the location of your Android SDK.

By default our build system will attempt to compile the Java bindings. To do so, the environment
variable `JAVA_HOME` should point to the location of your JDK.

When building for WebGL, you'll also need to set `EMSDK`. See [WebAssembly](#webassembly).

### IDE

We recommend using CLion to develop for Filament. Simply open the root directory's CMakeLists.txt
in CLion to obtain a usable project.

### Easy build

Once the required OS specific dependencies listed below are installed, you can use the script
located in `build.sh` to build Filament easily on macOS and Linux.

This script can be invoked from anywhere and will produce build artifacts in the `out/` directory
inside the Filament source tree.

To trigger an incremental debug build:

```
$ ./build.sh debug
```

To trigger an incremental release build:

```
$ ./build.sh release
```

To trigger both incremental debug and release builds:

```
$ ./build.sh debug release
```

To install the libraries and executables in `out/debug/` and `out/release/`, add the `-i` flag.
You can force a clean build by adding the `-c` flag. The script offers more features described
by executing `build.sh -h`.

### Disabling Java builds

By default our build system will attempt to compile the Java bindings. If you wish to skip this
compilation step simply pass the `-j` flag to `build.sh`:

```
$ ./build.sh -j release
```

If you use CMake directly instead of the build script, pass `-DENABLE_JAVA=OFF` to CMake instead.

### Linux

Make sure you've installed the following dependencies:

- `clang-7` or higher
- `libglu1-mesa-dev`
- `libc++-7-dev` (`libcxx-devel` and `libcxx-static` on Fedora) or higher
- `libc++abi-7-dev` (`libcxxabi-static` on Fedora) or higher
- `ninja-build`
- `libxi-dev`

After dependencies have been installed, we highly recommend using the [easy build](#easy-build)
script.

If you'd like to run `cmake` directly rather than using the build script, it can be invoked as
follows, with some caveats that are explained further down.

```
$ mkdir out/cmake-release
$ cd out/cmake-release
$ cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../release/filament ../..
```

Your Linux distribution might default to `gcc` instead of `clang`, if that's the case invoke
`cmake` with the following command:

```
$ mkdir out/cmake-release
$ cd out/cmake-release
# Or use a specific version of clang, for instance /usr/bin/clang-7
$ CC=/usr/bin/clang CXX=/usr/bin/clang++ CXXFLAGS=-stdlib=libc++ \
    cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../release/filament ../..
```

You can also export the `CC` and `CXX` environment variables to always point to `clang`. Another
solution is to use `update-alternatives` to both change the default compiler, and point to a
specific version of clang:

```
$ update-alternatives --install /usr/bin/clang clang /usr/bin/clang-7 100
$ update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-7 100
$ update-alternatives --install /usr/bin/cc cc /usr/bin/clang 100
$ update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++ 100
```

Finally, invoke `ninja`:

```
$ ninja
```

This will build Filament, its tests and samples, and various host tools.

### macOS

To compile Filament you must have the most recent version of Xcode installed and you need to
make sure the command line tools are setup by running:

```
$ xcode-select --install
```

After installing Java 1.8 you must also ensure that your `JAVA_HOME` environment variable is
properly set. If it doesn't already point to the appropriate JDK, you can simply add the following
to your `.profile`:

```
export JAVA_HOME="$(/usr/libexec/java_home)"
```

Then run `cmake` and `ninja` to trigger a build:

```
$ mkdir out/cmake-release
$ cd out/cmake-release
$ cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../release/filament ../..
$ ninja
```

### iOS

The easiest way to build Filament for iOS is to use `build.sh` and the
`-p ios` flag. For instance to build the debug target:

```
$ ./build.sh -p ios debug
```

See [ios/samples/README.md](./ios/samples/README.md) for more information.

### Windows

The following instructions have been tested on a machine running Windows 10. They should take you
from a machine with only the operating system to a machine able to build and run Filament.

Google employees require additional steps which can be found here [go/filawin](http://go/filawin).

Install the following components:

- [Windows 10 SDK](https://developer.microsoft.com/en-us/windows/downloads/windows-10-sdk)
- [Visual Studio 2015 or 2017](https://www.visualstudio.com/downloads)
- [Clang 7](http://releases.llvm.org/download.html)
- [Python 3.7](https://www.python.org/ftp/python/3.7.0/python-3.7.0.exe)
- [Cmake 3.13 or later](https://github.com/Kitware/CMake/releases/download/v3.13.4/cmake-3.13.4-win64-x64.msi)

If you're using Visual Studio 2017, you'll also need to install the [LLVM Compiler
Toolchain](https://marketplace.visualstudio.com/items?itemName=LLVMExtensions.llvm-toolchain)
extension.

Open an appropriate Native Tools terminal for the version of Visual Studio you are using:
- VS 2015: VS2015 x64 Native Tools Command Prompt
- VS 2017: x64 Native Tools Command Prompt for VS 2017

You can find these by clicking the start button and typing "x64 native tools".

Create a working directory:
```
> mkdir out/cmake-release
> cd out/cmake-release
```

Create the msBuild project:
```
# Visual Studio 2015:
> cmake -T"LLVM-vs2014" -G "Visual Studio 14 2015 Win64" ../..

# Visual Studio 2017
> cmake ..\.. -T"LLVM" -G "Visual Studio 15 2017 Win64" ^
-DCMAKE_CXX_COMPILER:PATH="C:\Program Files\LLVM\bin\clang-cl.exe" ^
-DCMAKE_C_COMPILER:PATH="C:\Program Files\LLVM\bin\clang-cl.exe" ^
-DCMAKE_LINKER:PATH="C:\Program Files\LLVM\bin\lld-link.exe"
```

Check out the output and make sure Clang for Windows frontend was found. You should see a line
showing the following output. Note that for Visual Studio 2017 this line may list Microsoft's
compiler, but the build will still in fact use Clang and you can proceed.

```
Clang:C:/Program Files/LLVM/msbuild-bin/cl.exe
```

You are now ready to build:
```
> msbuild  TNT.sln /t:material_sandbox /m /p:configuration=Release
```

Run it:
```
> samples\Release\material_sandbox.exe ..\..\assets\models\monkey\monkey.obj
```

#### Tips

- To troubleshoot an issue, use verbose mode via `/v:d` flag.
- To build a specific project, use `/t:NAME` flag (e.g: `/t:material_sandbox`).
- To build using more than one core, use parallel build flag: `/m`.
- To build a specific profile, use `/p:configuration=` (e.g: `/p:configuration=Debug`,
  `/p:configuration=Release`, and `/p:configuration=RelWithDebInfo`).
- The msBuild project is what is used by Visual Studio behind the scene to build. Building from VS
  or from the command-line is the same thing.

#### Building with Ninja on Windows

Alternatively, you can use [Ninja](https://ninja-build.org/) to build for Windows. An MSVC
installation is still necessary.

First, install the dependencies listed under [Windows](#Windows) as well as Ninja. Then open up a
Native Tools terminal as before. Create a build directory inside Filament and run the
following CMake command:

```
> cmake .. -G Ninja ^
-DCMAKE_CXX_COMPILER:PATH="C:\Program Files\LLVM\bin\clang-cl.exe" ^
-DCMAKE_C_COMPILER:PATH="C:\Program Files\LLVM\bin\clang-cl.exe" ^
-DCMAKE_LINKER:PATH="C:\Program Files\LLVM\bin\lld-link.exe" ^
-DCMAKE_BUILD_TYPE=Release
```

You should then be able to build by invoking Ninja:

```
> ninja
```

#### Development tips

- Before shipping a binary, make sure you used Release profile and make sure you have no libc/libc++
  dependencies with [Dependency Walker](http://www.dependencywalker.com).
- Application Verifier and gflags.exe can be of great help to trackdown heap corruption. Application
  Verifier is easy to setup with a GUI. For gflags, use: `gflags /p /enable pheap-buggy.exe`.

#### Running a simple test

To confirm Filament was properly built, run the following command from the build directory:

```
> samples\material_sandbox.exe --ibl=..\..\samples\envs\pillars ..\..\assets\models\sphere\sphere.obj
```

### Android

Before building Filament for Android, make sure to build Filament for your host. Some of the
host tools are required to successfully build for Android.

Filament can be built for the following architectures:

- ARM 64-bit (`arm64-v8a`)
- ARM 32-bit (`armeabi-v7a`)
- Intel 64-bit (`x86_64`)
- Intel 32-bit (`x86`)

Note that the main target is the ARM 64-bit target. Our implementation is optimized first and
foremost for `arm64-v8a`.

To build Android on Windows machines, see [android/Windows.md](android/Windows.md).

#### Easy Android build

The easiest way to build Filament for Android is to use `build.sh` and the
`-p android` flag. For instance to build the release target:

```
$ ./build.sh -p android release
```

Run `build.sh -h` for more information.

#### ARM 64-bit target (arm64-v8a)

Then invoke CMake in a build directory of your choice, inside of filament's directory:

```
$ mkdir out/android-build-release-aarch64
$ cd out/android-build-release-aarch64
$ cmake -G Ninja -DCMAKE_TOOLCHAIN_FILE=../../build/toolchain-aarch64-linux-android.cmake \
        -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../android-release/filament ../..
```

And then invoke `ninja`:

```
$ ninja install
```

or

```
$ ninja install/strip
```

This will generate Filament's Android binaries in `out/android-release`. This location is important
to build the Android Studio projects located in `filament/android`. After install, the library
binaries should be found in `out/android-release/filament/lib/arm64-v8a`.

#### ARM 32-bit target (armeabi-v7a)

Then invoke CMake in a build directory of your choice, inside of filament's directory:

```
$ mkdir out/android-build-release-arm
$ cd out/android-build-release-arm
$ cmake -G Ninja -DCMAKE_TOOLCHAIN_FILE=../../build/toolchain-arm7-linux-android.cmake \
        -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../android-release/filament ../..
```

And then invoke `ninja`:

```
$ ninja install
```

or

```
$ ninja install/strip
```

This will generate Filament's Android binaries in `out/android-release`. This location is important
to build the Android Studio projects located in `filament/android`. After install, the library
binaries should be found in `out/android-release/filament/lib/armeabi-v7a`.

#### Intel 64-bit target (x86_64)

Then invoke CMake in a build directory of your choice, sibling of filament's directory:

```
$ mkdir out/android-build-release-x86_64
$ cd out/android-build-release-x86_64
$ cmake -G Ninja -DCMAKE_TOOLCHAIN_FILE=../../filament/build/toolchain-x86_64-linux-android.cmake \
        -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../out/android-release/filament ../..
```

And then invoke `ninja`:

```
$ ninja install
```

or

```
$ ninja install/strip
```

This will generate Filament's Android binaries in `out/android-release`. This location is important
to build the Android Studio projects located in `filament/android`. After install, the library
binaries should be found in `out/android-release/filament/lib/x86_64`.

#### Intel 32-bit target (x86)

Then invoke CMake in a build directory of your choice, sibling of filament's directory:

```
$ mkdir out/android-build-release-x86
$ cd out/android-build-release-x86
$ cmake -G Ninja -DCMAKE_TOOLCHAIN_FILE=../../filament/build/toolchain-x86-linux-android.cmake \
        -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../out/android-release/filament ../..
```

And then invoke `ninja`:

```
$ ninja install
```

or

```
$ ninja install/strip
```

This will generate Filament's Android binaries in `out/android-release`. This location is important
to build the Android Studio projects located in `filament/android`. After install, the library
binaries should be found in `out/android-release/filament/lib/x86`.

### AAR

Before you attempt to build the AAR, make sure you've compiled and installed the native libraries
as explained in the sections above. You must have the following ABIs built in
`out/android-release/filament/lib/`:

- `arm64-v8a`
- `armeabi-v7a`
- `x86_64`
- `x86`

To build Filament's AAR simply open the Android Studio project in `android/filament-android`. The
AAR is a universal AAR that contains all supported build targets:

- `arm64-v8a`
- `armeabi-v7a`
- `x86_64`
- `x86`

To filter out unneeded ABIs, rely on the `abiFilters` of the project that links against Filament's
AAR.

Alternatively you can build the AAR from the command line by executing the following the
`android/filament-android` directory:

```
$ ./gradlew -Pfilament_dist_dir=../../out/android-release/filament assembleRelease
```

The `-Pfilament_dist_dir` can be used to specify a different installation directory (it must match
the CMake install prefix used in the previous steps).

### Using Filament's AAR

Create a new module in your project and select _Import .JAR or .AAR Package_ when prompted. Make
sure to add the newly created module as a dependency to your application.

If you do not wish to include all supported ABIs, make sure to create the appropriate flavors in
your Gradle build file. For example:

```
flavorDimensions 'cpuArch'
productFlavors {
    arm8 {
        dimension 'cpuArch'
        ndk {
            abiFilters 'arm64-v8a'
        }
    }
    arm7 {
        dimension 'cpuArch'
        ndk {
            abiFilters 'armeabi-v7a'
        }
    }
    x86_64 {
        dimension 'cpuArch'
        ndk {
            abiFilters 'x86_64'
        }
    }
    x86 {
        dimension 'cpuArch'
        ndk {
            abiFilters 'x86'
        }
    }
    universal {
        dimension 'cpuArch'
    }
}
```

### WebAssembly

The core Filament library can be cross-compiled to WebAssembly from either macOS or Linux. To get
started, follow the instructions for building Filament on your platform ([macOS](#macos) or
[linux](#linux)), which will ensure you have the proper dependencies installed.

Next, you need to install the Emscripten SDK. The following instructions show how to install the
same version that our continuous builds use.

```
cd <your chosen parent folder for the emscripten SDK>
curl -L https://github.com/emscripten-core/emsdk/archive/a77638d.zip > emsdk.zip
unzip emsdk.zip
mv emsdk-* emsdk
cd emsdk
./emsdk update
./emsdk install sdk-1.38.28-64bit
./emsdk activate sdk-1.38.28-64bit
```

After this you can invoke the [easy build](#easy-build) script as follows:

```
export EMSDK=<your chosen home for the emscripten SDK>
./build.sh -p webgl release
```

The EMSDK variable is required so that the build script can find the Emscripten SDK. The build
creates a `samples` folder that can be used as the root of a simple static web server. Note that you
cannot open the HTML directly from the filesystem due to CORS. One way to deal with this is to
use Python to create a quick localhost server:

```
cd out/cmake-webgl-release/web/samples
python3 -m http.server     # Python 3
python -m SimpleHTTPServer # Python 2.7
```

You can then open http://localhost:8000/suzanne.html in your web browser.

Alternatively, if you have node installed you can use the
[live-server](https://www.npmjs.com/package/live-server) package, which automatically refreshes the
web page when it detects a change.

Each sample app has its own handwritten html file. Additionally the server folder contains assets
such as meshes, textures, and materials.

## Running the native samples

The `samples/` directory contains several examples of how to use Filament with SDL2.

Some of the samples accept FBX/OBJ meshes while others rely on the `filamesh` file format. To
generate a `filamesh ` file from an FBX/OBJ asset, run the `filamesh` tool
(`./tools/filamesh/filamesh` in your build directory):

```
filamesh ./assets/models/monkey/monkey.obj monkey.filamesh
```

Most samples accept an IBL that must be generated using the `cmgen` tool (`./tools/filamesh/cmgen`
in your build directory). These sample apps expect a path to a directory containing the '.rgb32f'
files for the IBL (which are PNGs containing `R11F_G11F_B10F` data). To generate an IBL simply use
this command:

```
cmgen -x ./ibls/ my_ibl.exr
```

The source environment map can be a PNG (8 or 16 bit), a PSD (16 or 32 bit), an HDR or an OpenEXR
file. The environment map can be an equirectangular projection, a horizontal cross, a vertical
cross, or a list of cubemap faces (horizontal or vertical).

`cmgen` will automatically create a directory based on the name of the source environment map. In
the example above, the final directory will be `./ibls/my_ibl/`. This directory should contain the
pre-filtered environment map (one file per cubemap face and per mip level), the environment map
texture for the skybox and a text file containing the spherical harmonics for indirect diffuse
lighting.

If you prefer a blurred background, run `cmgen` with this flag: `--extract-blur=0.1`. The numerical
value is the desired roughness between 0 and 1.

## Rendering with Filament

### Native Linux, macOS and Windows

You must create an `Engine`, a `Renderer` and a `SwapChain`. The `SwapChain` is created from a
native window pointer (an `NSView` on macOS or a `HWND` on Windows for instance):

```c++
Engine* engine = Engine::create();
SwapChain* swapChain = engine->createSwapChain(nativeWindow);
Renderer* renderer = engine->createRenderer();
```

To render a frame you must then create a `View`, a `Scene` and a `Camera`:

```c++
Camera* camera = engine->createCamera();
View* view = engine->createView();
Scene* scene = engine->createScene();

view->setCamera(camera);
view->setScene(scene);
```

Renderables are added to the scene:

```c++
Entity renderable = EntityManager::get().create();
// build a quad
RenderableManager::Builder(1)
        .boundingBox({{ -1, -1, -1 }, { 1, 1, 1 }})
        .material(0, materialInstance)
        .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, vertexBuffer, indexBuffer, 0, 6)
        .culling(false)
        .build(*engine, renderable);
scene->addEntity(renderable);
```

The material instance is obtained from a material, itself loaded from a binary blob generated
by `matc`:

```c++
Material* material = Material::Builder()
        .package((void*) BAKED_MATERIAL_PACKAGE, sizeof(BAKED_MATERIAL_PACKAGE))
        .build(*engine);
MaterialInstance* materialInstance = material->createInstance();
```

To learn more about materials and `matc`, please refer to the
[materials documentation](./docs/Materials.md.html).

To render, simply pass the `View` to the `Renderer`:

```c++
// beginFrame() returns false if we need to skip a frame
if (renderer->beginFrame(swapChain)) {
    // for each View
    renderer->render(view);
    renderer->endFrame();
}
```

For complete examples of Linux, macOS and Windows Filament applications, look at the source files
in the `samples/` directory. These samples are all based on `samples/app/` which contains the code
that creates a native window with SDL2 and initializes the Filament engine, renderer and views.

### Java on Linux, macOS and Windows

After building Filament, you can use `filament-java.jar` and its companion `filament-jni` native
library to use Filament in desktop Java applications.

You must always first initialize Filament by calling `Filament.init()`.

You can use Filament either with AWT or Swing, using respectively a `FilamentCanvas` or a
`FilamentPanel`.

Following the steps above (how to use Filament from native code), create an `Engine` and a
`Renderer`, but instead of calling `beginFrame` and `endFrame` on the renderer itself, call
these methods on `FilamentCanvas` or `FilamentPanel`.

### Android

See `android/samples` for examples of how to use Filament on Android.

You must always first initialize Filament by calling `Filament.init()`.

Rendering with Filament on Android is similar to rendering from native code (the APIs are largely
the same across languages). You can render into a `Surface` by passing a `Surface` to the
`createSwapChain` method. This allows you to render to a `SurfaceTexture`, a `TextureView` or
a `SurfaceView`. To make things easier we provide an Android specific API called `UiHelper` in the
package `com.google.android.filament.android`. All you need to do is set a render callback on the
helper and attach your `SurfaceView` or `TextureView` to it. You are still responsible for
creating the swap chain in the `onNativeWindowChanged()` callback.

### iOS

See `ios/samples` for examples of using Filament on iOS.

Filament on iOS is largely the same as native rendering with C++. A `CAEAGLLayer` or `CAMetalLayer`
is passed to the `createSwapChain` method. Filament for iOS supports both OpenGL ES and Vulkan via
MoltenVK.

## Generating C++ documentation

To generate the documentation you must first install `doxygen` and `graphviz`, then run the 
following commands:

```
$ cd filament/filament
$ doxygen docs/doxygen/filament.doxygen
```

Finally simply open `docs/html/index.html` in your web browser.

## Assets

To get started you can use the textures and environment maps found respectively in
`third_party/textures` and `third_party/environments`. These assets are under CC0 license. Please
refer to their respective `URL.txt` files to know more about the original authors.

## Dependencies

One of our design goals is that Filament itself should have no dependencies or as few dependencies
as possible. The current external dependencies of the runtime library include:

- STL
- robin-map (header only library)

When building with Vulkan enabled, we have a few additional small dependencies:

- vkmemalloc
- smol-v

Host tools (such as `matc` or `cmgen`) can use external dependencies freely.

## How to make contributions

Please read and follow the steps in [CONTRIBUTING.md](/CONTRIBUTING.md). Make sure you are
familiar with the [code style](/CODE_STYLE.md).

## License


Apache License
                           Version 2.0, January 2004
                        https://www.apache.org/licenses/

   TERMS AND CONDITIONS FOR USE, REPRODUCTION, AND DISTRIBUTION

   1. Definitions.

      "License" shall mean the terms and conditions for use, reproduction,
      and distribution as defined by Sections 1 through 9 of this document.

      "Licensor" shall mean the copyright owner or entity authorized by
      the copyright owner that is granting the License.

      "Legal Entity" shall mean the union of the acting entity and all
      other entities that control, are controlled by, or are under common
      control with that entity. For the purposes of this definition,
      "control" means (i) the power, direct or indirect, to cause the
      direction or management of such entity, whether by contract or
      otherwise, or (ii) ownership of fifty percent (50%) or more of the
      outstanding shares, or (iii) beneficial ownership of such entity.

      "You" (or "Your") shall mean an individual or Legal Entity
      exercising permissions granted by this License.

      "Source" form shall mean the preferred form for making modifications,
      including but not limited to software source code, documentation
      source, and configuration files.

      "Object" form shall mean any form resulting from mechanical
      transformation or translation of a Source form, including but
      not limited to compiled object code, generated documentation,
      and conversions to other media types.

      "Work" shall mean the work of authorship, whether in Source or
      Object form, made available under the License, as indicated by a
      copyright notice that is included in or attached to the work
      (an example is provided in the Appendix below).

      "Derivative Works" shall mean any work, whether in Source or Object
      form, that is based on (or derived from) the Work and for which the
      editorial revisions, annotations, elaborations, or other modifications
      represent, as a whole, an original work of authorship. For the purposes
      of this License, Derivative Works shall not include works that remain
      separable from, or merely link (or bind by name) to the interfaces of,
      the Work and Derivative Works thereof.

      "Contribution" shall mean any work of authorship, including
      the original version of the Work and any modifications or additions
      to that Work or Derivative Works thereof, that is intentionally
      submitted to Licensor for inclusion in the Work by the copyright owner
      or by an individual or Legal Entity authorized to submit on behalf of
      the copyright owner. For the purposes of this definition, "submitted"
      means any form of electronic, verbal, or written communication sent
      to the Licensor or its representatives, including but not limited to
      communication on electronic mailing lists, source code control systems,
      and issue tracking systems that are managed by, or on behalf of, the
      Licensor for the purpose of discussing and improving the Work, but
      excluding communication that is conspicuously marked or otherwise
      designated in writing by the copyright owner as "Not a Contribution."

      "Contributor" shall mean Licensor and any individual or Legal Entity
      on behalf of whom a Contribution has been received by Licensor and
      subsequently incorporated within the Work.

   2. Grant of Copyright License. Subject to the terms and conditions of
      this License, each Contributor hereby grants to You a perpetual,
      worldwide, non-exclusive, no-charge, royalty-free, irrevocable
      copyright license to reproduce, prepare Derivative Works of,
      publicly display, publicly perform, sublicense, and distribute the
      Work and such Derivative Works in Source or Object form.

   3. Grant of Patent License. Subject to the terms and conditions of
      this License, each Contributor hereby grants to You a perpetual,
      worldwide, non-exclusive, no-charge, royalty-free, irrevocable
      (except as stated in this section) patent license to make, have made,
      use, offer to sell, sell, import, and otherwise transfer the Work,
      where such license applies only to those patent claims licensable
      by such Contributor that are necessarily infringed by their
      Contribution(s) alone or by combination of their Contribution(s)
      with the Work to which such Contribution(s) was submitted. If You
      institute patent litigation against any entity (including a
      cross-claim or counterclaim in a lawsuit) alleging that the Work
      or a Contribution incorporated within the Work constitutes direct
      or contributory patent infringement, then any patent licenses
      granted to You under this License for that Work shall terminate
      as of the date such litigation is filed.

   4. Redistribution. You may reproduce and distribute copies of the
      Work or Derivative Works thereof in any medium, with or without
      modifications, and in Source or Object form, provided that You
      meet the following conditions:

      (a) You must give any other recipients of the Work or
          Derivative Works a copy of this License; and

      (b) You must cause any modified files to carry prominent notices
          stating that You changed the files; and

      (c) You must retain, in the Source form of any Derivative Works
          that You distribute, all copyright, patent, trademark, and
          attribution notices from the Source form of the Work,
          excluding those notices that do not pertain to any part of
          the Derivative Works; and

      (d) If the Work includes a "NOTICE" text file as part of its
          distribution, then any Derivative Works that You distribute must
          include a readable copy of the attribution notices contained
          within such NOTICE file, excluding those notices that do not
          pertain to any part of the Derivative Works, in at least one
          of the following places: within a NOTICE text file distributed
          as part of the Derivative Works; within the Source form or
          documentation, if provided along with the Derivative Works; or,
          within a display generated by the Derivative Works, if and
          wherever such third-party notices normally appear. The contents
          of the NOTICE file are for informational purposes only and
          do not modify the License. You may add Your own attribution
          notices within Derivative Works that You distribute, alongside
          or as an addendum to the NOTICE text from the Work, provided
          that such additional attribution notices cannot be construed
          as modifying the License.

      You may add Your own copyright statement to Your modifications and
      may provide additional or different license terms and conditions
      for use, reproduction, or distribution of Your modifications, or
      for any such Derivative Works as a whole, provided Your use,
      reproduction, and distribution of the Work otherwise complies with
      the conditions stated in this License.

   5. Submission of Contributions. Unless You explicitly state otherwise,
      any Contribution intentionally submitted for inclusion in the Work
      by You to the Licensor shall be under the terms and conditions of
      this License, without any additional terms or conditions.
      Notwithstanding the above, nothing herein shall supersede or modify
      the terms of any separate license agreement you may have executed
      with Licensor regarding such Contributions.

   6. Trademarks. This License does not grant permission to use the trade
      names, trademarks, service marks, or product names of the Licensor,
      except as required for reasonable and customary use in describing the
      origin of the Work and reproducing the content of the NOTICE file.

   7. Disclaimer of Warranty. Unless required by applicable law or
      agreed to in writing, Licensor provides the Work (and each
      Contributor provides its Contributions) on an "AS IS" BASIS,
      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
      implied, including, without limitation, any warranties or conditions
      of TITLE, NON-INFRINGEMENT, MERCHANTABILITY, or FITNESS FOR A
      PARTICULAR PURPOSE. You are solely responsible for determining the
      appropriateness of using or redistributing the Work and assume any
      risks associated with Your exercise of permissions under this License.

   8. Limitation of Liability. In no event and under no legal theory,
      whether in tort (including negligence), contract, or otherwise,
      unless required by applicable law (such as deliberate and grossly
      negligent acts) or agreed to in writing, shall any Contributor be
      liable to You for damages, including any direct, indirect, special,
      incidental, or consequential damages of any character arising as a
      result of this License or out of the use or inability to use the
      Work (including but not limited to damages for loss of goodwill,
      work stoppage, computer failure or malfunction, or any and all
      other commercial damages or losses), even if such Contributor
      has been advised of the possibility of such damages.

   9. Accepting Warranty or Additional Liability. While redistributing
      the Work or Derivative Works thereof, You may choose to offer,
      and charge a fee for, acceptance of support, warranty, indemnity,
      or other liability obligations and/or rights consistent with this
      License. However, in accepting such obligations, You may act only
      on Your own behalf and on Your sole responsibility, not on behalf
      of any other Contributor, and only if You agree to indemnify,
      defend, and hold each Contributor harmless for any liability
      incurred by, or claims asserted against, such Contributor by reason
      of your accepting any such warranty or additional liability.

   END OF TERMS AND CONDITIONS

   APPENDIX: How to apply the Apache License to your work.

      To apply the Apache License to your work, attach the following
      boilerplate notice, with the fields enclosed by brackets "[]"
      replaced with your own identifying information. (Don't include
      the brackets!)  The text should be enclosed in the appropriate
      comment syntax for the file format. We also recommend that a
      file or class name and description of purpose be included on the
      same "printed page" as the copyright notice for easier
      identification within third-party archives.

   Copyright 2019 Rolando Gopez Lacuata.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       https://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

## Disclaimer

This is not an officially supported Google product.
