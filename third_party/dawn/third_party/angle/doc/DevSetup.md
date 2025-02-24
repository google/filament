# ANGLE Development

ANGLE provides OpenGL ES 3.1 and EGL 1.5 libraries and tests. You can use these to build and run OpenGL ES applications on Windows, Linux, Mac and Android.

## Development setup

### Version Control

ANGLE uses git for version control. Helpful documentation can be found at [http://git-scm.com/documentation](http://git-scm.com/documentation).

### Required First Setup (do this first)

Note: If you are building inside a Chromium checkout [see these instructions instead](https://chromium.googlesource.com/angle/angle/+/HEAD/doc/BuildingAngleForChromiumDevelopment.md).

Required on all platforms:

 * [Python 3](https://www.python.org/downloads/) must be available in your path.
 * [depot_tools](https://commondatastorage.googleapis.com/chrome-infra-docs/flat/depot_tools/docs/html/depot_tools_tutorial.html#_setting_up)
   * Required to download dependencies (with gclient), generate build files (with GN), and compile ANGLE (with ninja).
   * Ensure `depot_tools` is in your path as it provides ninja for compilation.
 * For Googlers, run `download_from_google_storage --config` to login to Google Storage before fetching the source.

On Windows:

 * ***IMPORTANT: Set `DEPOT_TOOLS_WIN_TOOLCHAIN=0` in your environment if you are not a Googler.***
 * Install [Visual Studio Community 2022](https://visualstudio.microsoft.com/vs/)
 * Install the [Windows SDK](https://developer.microsoft.com/en-us/windows/downloads/sdk-archive/).
   * You can install it through Visual Studio Installer if available. It might be necessary to switch to the `Individual components` tab to find the latest version.
   * The currently supported Windows SDK version can be found in [vs_toolchain.py](https://chromium.googlesource.com/chromium/src/build/+/refs/heads/main/vs_toolchain.py).
   * The SDK is required for GN-generated Visual Studio projects, the D3D Debug runtime, and the latest HLSL Compiler runtime.
 * (optional) See the [Chromium Windows build instructions](https://chromium.googlesource.com/chromium/src/+/main/docs/windows_build_instructions.md) for more info.

On Linux:

 * Dependencies will be handled later (see `install-build-deps.sh` below).

On MacOS:

 * [XCode](https://developer.apple.com/xcode/) for Clang and development files.
 * For Googlers on MacOS, you'll first need authorization to download macOS SDK's from Chromium
   servers before running `gclient sync`. Obtain this authorization via `cipd auth-login`
   and following the instructions.

### Getting the source

```
mkdir angle
cd angle
fetch angle
```

If you're contributing code, you will also need to set up the Git `commit-msg` hook. See [ContributingCode#getting-started-with-gerrit](ContributingCode.md#getting-started-with-gerrit) for instructions.

On Linux only, you need to install all the necessary dependencies before going further by running this command:
```
./build/install-build-deps.sh
```

If building for Android (which requires Linux), switch to the [Android steps](https://chromium.googlesource.com/angle/angle.git/+/HEAD/doc/DevSetupAndroid.md) at this point.

After this completes successfully, you are ready to generate the ninja files:
```
gn gen out/Debug
```

If you had trouble checking out the code, please inspect the error message. As
a reminder, on Windows, ensure you **set `DEPOT_TOOLS_WIN_TOOLCHAIN=0` in
your environment if you are not a Googler**. If you are a Googler, ensure you
ran `download_from_google_storage --config`.

GN will generate ninja files. The default build options build ANGLE with clang
and in release mode. Often, the default options are the desired ones, but
they can be changed by running `gn args out/Debug`. Some options that are
commonly overriden for development are:

```
is_component_build = true/false      (false forces static links of dependencies)
target_cpu = "x64"/"x86"             (the default is "x64")
is_debug = true/false                (use false for release builds. is_debug = true is the default)
angle_assert_always_on = true/false  (enables release asserts and runtime debug layers)
is_clang = false (NOT RECOMMENDED)   (to use system default compiler instead of clang)
```

For a release build run `gn args out/Release` and set `is_debug = false`.
Optionally set `angle_assert_always_on = true` for Release testing.

On Windows, you can build for the Universal Windows Platform (UWP) or WinUI 3.
For UWP, set `target_os = "winuwp"` in the args. For WinUI 3, instead set
`angle_is_winappsdk=true` along with the path to the Windows App SDK
headers: `winappsdk_dir="/path/to/headers"`. The headers need to be generated
from the winmd files, which is done by running the `scripts/winappsdk_setup.py`
script and passing in the path to store the headers.
For both UWP and WinUI 3, setting `is_component_build = false` is highly
recommended to support moving libEGL.dll and libGLESv2.dll to an application's
directory and being self-contained, instead of depending on other DLLs
(d3dcompiler_47.dll is still needed for the Direct3D backend).
We also recommend using `is_clang = false`.

For more information on GN run `gn help`.

Use `autoninja` to compile on all platforms with one of the following commands:

```
autoninja -C out/Debug
autoninja -C out/Release
```

`depot_tools` provides `autoninja`, so it should be available in your path
from earlier steps. Ninja automatically calls GN to regenerate the build
files on any configuration change. `autoninja` automatically specifies a
thread count to `ninja` based on your system configuration.

### Building with Reclient (Google employees only)

Reclient is the recommended distributed compiler service to build ANGLE faster.

Step 1. Follow [Setup remote execution](https://g3doc.corp.google.com/company/teams/chrome/linux_build_instructions.md?cl=head#setup-remote-execution)
to download the required configuration, and complete the authentication.

To download the required configuration:

In .gclient, add `"download_remoteexec_cfg: True,"` in custom_vars:

```
solutions = [
  {
    # some other args
    "custom_vars": {
        "download_remoteexec_cfg": True,
    },
  },
]

```

Then run

```
gclient sync
```

To complete authentication:

1. Install gcloud SDK go/gcloud-cli#installing-and-using-the-cloud-sdk.
Make sure the gcloud tool is available on your `$PATH`.

2. Log into gcloud with your @google.com account:

```
gcloud auth login
```

If asked for a project ID, enter "0".

Step 2. Enable the usage of reclient by adding below content in GN arg:

```
use_remoteexec = true
```

### Building and Debugging with Visual Studio

To generate the Visual Studio solution in `out/Debug/angle-debug.sln`:

```
gn gen out/Debug --sln=angle-debug --ide=vs2022 --ninja-executable="C:\src\angle\third_party\ninja\ninja.exe"
```

In Visual Studio:
 1. Open the ANGLE solution file `out/Debug/angle-debug.sln`.
 2. We recommended you use `autoninja` from a command line to build manually.
 3. "Build Solution" from the IDE is broken with GN. You can use the IDE to build one target or one file at a time.

Once the build completes, all ANGLE libraries, tests, and samples will be located in `out/Debug`.

### Building ANGLE for Android

See the Android specific [documentation](DevSetupAndroid.md#ANGLE-for-Android).

### Building ANGLE for iOS simulator

This is currently possible only from Chromium checkout.
Follow [Chromium for iOS build instructions](https://chromium.googlesource.com/chromium/src/+/HEAD/docs/ios/build_instructions.md).
GN args used by [ANGLE for iOS builder](https://ci.chromium.org/ui/p/chromium/builders/luci.chromium.ci/ios-angle-builder) are supported, e.g.:
```
dcheck_always_on = true
enable_run_ios_unittests_with_xctest = true
is_component_build = false
is_debug = false
symbol_level = 1
target_cpu = "x64"
target_environment = "simulator"
target_os = "ios"
```
Building `angle_end2end_tests` and `angle_white_box_tests` targets is supported.

## Application Development with ANGLE

This sections describes how to use ANGLE to build an OpenGL ES application.

### Choosing a Backend

ANGLE can use a variety of backing renderers based on platform.  On Windows, it defaults to D3D11 where it's available,
or D3D9 otherwise.  On other desktop platforms, it defaults to GL.  On mobile, it defaults to GLES.

ANGLE provides an EGL extension called `EGL_ANGLE_platform_angle` which allows uers to select
which renderer to use at EGL initialization time by calling eglGetPlatformDisplayEXT with special
enums. Details of the extension can be found in its specification in
`extensions/EGL_ANGLE_platform_angle.txt` and `extensions/EGL_ANGLE_platform_angle_*.txt` and
examples of its use can be seen in the ANGLE samples and tests, particularly `util/EGLWindow.cpp`.

To change the default D3D backend:

 1. Open `src/libANGLE/renderer/d3d/DisplayD3D.cpp`
 2. Locate the definition of `ANGLE_DEFAULT_D3D11` near the head of the file, and set it to your preference.

To remove any backend entirely:

 1. Run `gn args <path/to/build/dir>`
 2. Set the appropriate variable to `false`. Options are:
   - `angle_enable_d3d9`
   - `angle_enable_d3d11`
   - `angle_enable_gl`
   - `angle_enable_metal`
   - `angle_enable_null`
   - `angle_enable_vulkan`
   - `angle_enable_essl`
   - `angle_enable_glsl`

### To Use ANGLE in Your Application
On Windows:

 1. Configure your build environment to have access to the `include` folder to provide access to the standard Khronos EGL and GLES2 header files.
  * For Visual C++
     * Right-click your project in the _Solution Explorer_, and select _Properties_.
     * Under the _Configuration Properties_ branch, click _C/C++_.
     * Add the relative path to the Khronos EGL and GLES2 header files to _Additional Include Directories_.
 2. Configure your build environment to have access to `libEGL.lib` and `libGLESv2.lib` found in the build output directory (see [Building ANGLE](#building-with-visual-studio)).
   * For Visual C++
     * Right-click your project in the _Solution Explorer_, and select _Properties_.
     * Under the _Configuration Properties_ branch, open the _Linker_ branch and click _Input_.
     * Add the relative paths to both the `libEGL.lib` file and `libGLESv2.lib` file to _Additional Dependencies_, separated by a semicolon.
 3. Copy `libEGL.dll` and `libGLESv2.dll` from the build output directory (see [Building ANGLE](#building-with-visual-studio)) into your application folder.
 4. Code your application to the Khronos [OpenGL ES 2.0](http://www.khronos.org/registry/gles/) and [EGL 1.4](http://www.khronos.org/registry/egl/) APIs.

On Linux and MacOS, either:

 - Link you application against `libGLESv2` and `libEGL`
 - Use `dlopen` to load the OpenGL ES and EGL entry points at runtime.

## GLSL ES Translator

In addition to OpenGL ES and EGL libraries, ANGLE also provides a GLSL ES
translator. The translator targets various back-ends, including HLSL, GLSL
for desktop and mobile, SPIR-V and Metal SL. To build the translator, build
the `angle_shader_translator` target. Run the translator binary without
arguments to see a usage message.

### Source and Building

The translator code is included with ANGLE but fully independent; it resides
in [`src/compiler`](../src/compiler). Follow the steps above for
[getting and building ANGLE](#getting-the-source) to build the translator on
the platform of your choice.

### Usage

The ANGLE [`shader_translator`](../samples/shader_translator/shader_translator.cpp)
sample demos basic C++ API usage. To translate a GLSL ES shader, call the following
functions in the same order:

 * `sh::Initialize()` initializes the translator library and must be called only once from each process using the translator.
 * `sh::ContructCompiler()` creates a translator object for vertex or fragment shader.
 * `sh::Compile()` translates the given shader.
 * `sh::Destruct()` destroys the given translator.
 * `sh::Finalize()` shuts down the translator library and must be called only once from each process using the translator.

## OpenCL Support

A few GN args are needed to enable OpenCL runtime code to be built in the ANGLE lib(s).

`args.gn`
```
# Global enable flag for OpenCL support
angle_enable_cl = true

# Enable the Vulkan backend
angle_enable_vulkan = true

# Enable the CL backend (i.e. passthrough) if needed
angle_enable_cl_passthrough = false  // or true
```

### OpenCL artifacts

The two main artifacts generated here are `OpenCL_ANGLE` and `GLESv2`:

- `OpenCL_ANGLE` : Acts as a loader for CL entrypoints from the `GLESv2` library and populates it's
API dispatch table with them.
- `GLESv2` : Is the ANGLE library itself that also includes the OpenCL entrypoints/runtime when
`angle_enable_cl = true`.

Additional `Vulkan-backend` artifacts

- `clspv_core_shared` : clspv as a shared library to compile OpenCL C source over a
[C API](https://github.com/google/clspv/blob/main/docs/C_API.md) used by the `GLESv2` library.

### OpenCL Usage

ANGLE's OpenCL implementation acts no different from any other OpenCL ICD. Applications can either link to an
existing system OpenCL-ICD-Loader, or it can link directly to the `OpenCL_ANGLE` via its exported OpenCL
entrypoints.

If using an existing system OpenCL-ICD-Loader, then make sure `OpenCL_ANGLE` can be found by the OpenCL-ICD-Loader,
see [OpenCL-ICD-Loader](https://github.com/KhronosGroup/OpenCL-ICD-Loader) for details on this.

In both cases, `OpenCL_ANGLE` works by using `LoadLibrary/dlopen` on the `GLESv2` library to build the OpenCL
dispatch table using the entrypoints/symbols from `GLESv2` library. From then on, that API dispatch table is either
given to the system ICD Loader, or if app is linked directly to the `OpenCL_ANGLE` lib, it just uses its
singular dispatch table to forward onto `GLESv2` OpenCL entrypoints.
