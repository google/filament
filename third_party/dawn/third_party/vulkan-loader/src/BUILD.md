# Build Instructions

Instructions for building this repository on Linux, Windows, and MacOS.

## Table Of Contents

- [Build Instructions](#build-instructions)
  - [Table Of Contents](#table-of-contents)
  - [Contributing to the Repository](#contributing-to-the-repository)
  - [Repository Content](#repository-content)
    - [Installed Files](#installed-files)
  - [Build Requirements](#build-requirements)
    - [Test Requirements](#test-requirements)
  - [Repository Set-Up](#repository-set-up)
    - [Display Drivers](#display-drivers)
    - [Repository Dependencies](#repository-dependencies)
      - [Vulkan-Headers](#vulkan-headers)
      - [Test Dependencies](#test-dependencies)
    - [Warnings as errors off by default!](#warnings-as-errors-off-by-default)
    - [Build and Install Directory Locations](#build-and-install-directory-locations)
    - [Building Dependent Repositories with Known-Good Revisions](#building-dependent-repositories-with-known-good-revisions)
      - [Automatically](#automatically)
      - [Manually](#manually)
        - [Notes About the Manual Option](#notes-about-the-manual-option)
    - [Generated source code](#generated-source-code)
    - [Build Options](#build-options)
  - [Building On Windows](#building-on-windows)
    - [Windows Development Environment Requirements](#windows-development-environment-requirements)
    - [Windows Build - Microsoft Visual Studio](#windows-build---microsoft-visual-studio)
      - [Windows Quick Start](#windows-quick-start)
      - [Use `CMake` to Create the Visual Studio Project Files](#use-cmake-to-create-the-visual-studio-project-files)
      - [Build the Solution From the Command Line](#build-the-solution-from-the-command-line)
      - [Build the Solution With Visual Studio](#build-the-solution-with-visual-studio)
      - [Windows Install Target](#windows-install-target)
  - [Building On Linux](#building-on-linux)
    - [Linux Development Environment Requirements](#linux-development-environment-requirements)
      - [Required Package List](#required-package-list)
    - [Linux Build](#linux-build)
      - [Linux Quick Start](#linux-quick-start)
      - [Use CMake to Create the Make Files](#use-cmake-to-create-the-make-files)
      - [Build the Project](#build-the-project)
    - [Linux Notes](#linux-notes)
      - [WSI Support Build Options](#wsi-support-build-options)
      - [Linux Install to System Directories](#linux-install-to-system-directories)
      - [Linux 32-bit support](#linux-32-bit-support)
  - [Building on MacOS](#building-on-macos)
    - [MacOS Development Environment Requirements](#macos-development-environment-requirements)
    - [Clone the Repository](#clone-the-repository)
    - [MacOS build](#macos-build)
      - [Building with the Unix Makefiles Generator](#building-with-the-unix-makefiles-generator)
      - [Building with the Xcode Generator](#building-with-the-xcode-generator)
  - [Building on Fuchsia](#building-on-fuchsia)
    - [SDK Symbols](#sdk-symbols)
  - [Building on QNX](#building-on-qnx)
  - [Cross Compilation](#cross-compilation)
    - [Unknown function handling which requires explicit assembly implementations](#unknown-function-handling-which-requires-explicit-assembly-implementations)
      - [Platforms which fully support unknown function handling](#platforms-which-fully-support-unknown-function-handling)
  - [Tests](#tests)


## Contributing to the Repository

If you intend to contribute, the preferred work flow is for you to develop
your contribution in a fork of this repository in your GitHub account and then
submit a pull request. Please see the [CONTRIBUTING.md](CONTRIBUTING.md) file
in this repository for more details.

## Repository Content

This repository contains the source code necessary to build the desktop Vulkan
loader and its tests.

### Installed Files

The `install` target installs the following files under the directory
indicated by *install_dir*:

- *install_dir*`/lib` : The Vulkan loader library
- *install_dir*`/bin` : The Vulkan loader library DLL (Windows)

## Build Requirements

1. `C99` capable compiler
2. `CMake` version 3.22.1 or greater
3. `Git`

### Test Requirements

1. `C++17` capable compiler

## Repository Set-Up

### Display Drivers

This repository does not contain a Vulkan-capable driver. You will need to
obtain and install a Vulkan driver from your graphics hardware vendor or from
some other suitable source if you intend to run Vulkan applications.


### Repository Dependencies

This repository attempts to resolve some of its dependencies by using
components found from the following places, in this order:

1. CMake or Environment variable overrides (e.g., -D VULKAN_HEADERS_INSTALL_DIR)
2. System-installed packages, mostly applicable on Linux

Dependencies that cannot be resolved by the SDK or installed packages must be
resolved with the "install directory" override and are listed below. The
"install directory" override can also be used to force the use of a specific
version of that dependency.

#### Vulkan-Headers

This repository has a required dependency on the [Vulkan Headers repository](https://github.com/KhronosGroup/Vulkan-Headers).
The Vulkan-Headers repository contains the Vulkan API definition files that are
required to build the loader.

#### Test Dependencies

The loader tests depend on the [Google Test](https://github.com/google/googletest) library and
on Windows platforms depends on the [Microsoft Detours](https://github.com/microsoft/Detours) library.

To build the tests, pass both `-D UPDATE_DEPS=ON` and `-D BUILD_TESTS=ON` options when generating the project:
```bash
cmake ... -D UPDATE_DEPS=ON -D BUILD_TESTS=ON ...
```
This will ensure googletest and detours is downloaded and the appropriate version is used.

### Warnings as errors off by default!

By default `BUILD_WERROR` is `OFF`. The idiom for open source projects is to NOT enable warnings as errors.

System/language package managers have to build on multiple different platforms and compilers.

By defaulting to `ON` we cause issues for package managers since there is no standard way to disable warnings.

Add `-D BUILD_WERROR=ON` to your workflow

### Build and Install Directory Locations

A common convention is to place the `build` directory in the top directory of
the repository and place the `install` directory as a child of the `build`
directory. The remainder of these instructions follow this convention,
although you can place these directories in any location.

### Building Dependent Repositories with Known-Good Revisions

There is a Python utility script, `scripts/update_deps.py`, that you can use
to gather and build the dependent repositories mentioned above.
This program uses information stored in the `scripts/known-good.json` file
to checkout dependent repository revisions that are known to be compatible with
the revision of this repository that you currently have checked out.

You can choose to do this automatically or manually.
The first step to either is cloning the Vulkan-Loader repo and stepping into
that newly cloned folder:

```
  git clone git@github.com:KhronosGroup/Vulkan-Loader.git
  cd Vulkan-Loader
```

#### Automatically

On the other hand, if you choose to let the CMake scripts do all the
heavy-lifting, you may just trigger the following CMake commands:

```
  cmake -S . -B build -D UPDATE_DEPS=On
  cmake --build build
```

#### Manually

To manually update the dependencies you now must create the build folder, and
run the update deps script followed by the necessary CMake build commands:

```
  mkdir build
  cd build
  python ../scripts/update_deps.py
  cmake -C helper.cmake ..
  cmake --build .
```

##### Notes About the Manual Option

- You may need to adjust some of the CMake options based on your platform. See
  the platform-specific sections later in this document.
- The `update_deps.py` script fetches and builds the dependent repositories in
  the current directory when it is invoked.
- The `--dir` option for `update_deps.py` can be used to relocate the
  dependent repositories to another arbitrary directory using an absolute or
  relative path.
- The `update_deps.py` script generates a file named `helper.cmake` and places
  it in the same directory as the dependent repositories.
  This file contains CMake commands to set the CMake `*_INSTALL_DIR` variables
  that are used to point to the install artifacts of the dependent repositories.
  The `-C helper.cmake` option is used to set these variables when you generate
  the build files.
- If using "MINGW" (Git For Windows), you may wish to run
  `winpty update_deps.py` in order to avoid buffering all of the script's
  "print" output until the end and to retain the ability to interrupt script
  execution.
- Please use `update_deps.py --help` to list additional options and read the
  internal documentation in `update_deps.py` for further information.

### Generated source code

This repository contains generated source code in the `loader/generated`
directory which is not intended to be modified directly.
Instead, changes should be made to the corresponding generator in the `scripts` directory.
The source files can then be regenerated using `scripts/generate_source.py`.

Run `python scripts/generate_source.py --help` to see how to invoke it.

A helper CMake target `loader_codegen` is also provided to simplify the invocation of `scripts/generate_source.py`.

Note: By default this helper target is disabled. To enable it, add `-D LOADER_CODEGEN=ON`
to CMake, as shown below.

```
cmake -S . -B build -D LOADER_CODEGEN=ON
cmake --build . --target loader_codegen
```

### Build Options

When generating build files through CMake, several options can be specified to
customize the build.
The following is a table of all on/off options currently supported by this repository:

| Option                          | Platform      | Default | Description                                                                                                                                                                       |
| ------------------------------- | ------------- | ------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| BUILD_TESTS                     | All           | `OFF`   | Controls whether or not the loader tests are built.                                                                                                                               |
| BUILD_WSI_XCB_SUPPORT           | Linux         | `ON`    | Build the loader with the XCB entry points enabled. Without this, the XCB headers should not be needed, but the extension `VK_KHR_xcb_surface` won't be available.                |
| BUILD_WSI_XLIB_SUPPORT          | Linux         | `ON`    | Build the loader with the Xlib entry points enabled. Without this, the X11 headers should not be needed, but the extension `VK_KHR_xlib_surface` won't be available.              |
| BUILD_WSI_WAYLAND_SUPPORT       | Linux         | `ON`    | Build the loader with the Wayland entry points enabled. Without this, the Wayland headers should not be needed, but the extension `VK_KHR_wayland_surface` won't be available.    |
| BUILD_WSI_DIRECTFB_SUPPORT      | Linux         | `OFF`   | Build the loader with the DirectFB entry points enabled. Without this, the DirectFB headers should not be needed, but the extension `VK_EXT_directfb_surface` won't be available. |
| BUILD_WSI_SCREEN_QNX_SUPPORT    | QNX           | `OFF`   | Build the loader with the QNX Screen entry points enabled. Without this the extension `VK_QNX_screen_surface` won't be available.                                                 |
| ENABLE_WIN10_ONECORE            | Windows       | `OFF`   | Link the loader to the [OneCore](https://msdn.microsoft.com/en-us/library/windows/desktop/mt654039.aspx) umbrella library, instead of the standard Win32 ones.                    |
| USE_GAS                         | Linux         | `ON`    | Controls whether to build assembly files with the GNU assembler, else fallback to C code.                                                                                         |
| USE_MASM                        | Windows       | `ON`    | Controls whether to build assembly files with MS assembler, else fallback to C code                                                                                               |
| LOADER_ENABLE_ADDRESS_SANITIZER | Linux & macOS | `OFF`   | Enables Address Sanitizer in the loader and tests.                                                                                                                                |
| LOADER_ENABLE_THREAD_SANITIZER  | Linux & macOS | `OFF`   | Enables Thread Sanitizer in the loader and tests.                                                                                                                                 |
| LOADER_USE_UNSAFE_FILE_SEARCH   | All           | `OFF`   | Disables security policies that prevent unsecure locations from being used when running with elevated permissions.                                                                |
| LOADER_CODEGEN                  | All           | `OFF`   | Creates a helper CMake target to generate code.                                                                                                                                   |

NOTE: `LOADER_USE_UNSAFE_FILE_SEARCH` should NOT be enabled except in very specific contexts (like isolated test environments)!

The following is a table of all string options currently supported by this repository:

| Option                | Platform    | Default                       | Description                                                                                                                                          |
| --------------------- | ----------- | ----------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------- |
| FALLBACK_CONFIG_DIRS  | Linux/MacOS | `/etc/xdg`                    | Configuration path(s) to use instead of `XDG_CONFIG_DIRS` if that environment variable is unavailable. The default setting is freedesktop compliant. |
| FALLBACK_DATA_DIRS    | Linux/MacOS | `/usr/local/share:/usr/share` | Configuration path(s) to use instead of `XDG_DATA_DIRS` if that environment variable is unavailable. The default setting is freedesktop compliant.   |
| BUILD_DLL_VERSIONINFO | Windows     | `""` (empty string)           | Allows setting the Windows specific version information for the Loader DLL. Format is "major.minor.patch.build".                                     |

These variables should be set using the `-D` option when invoking CMake to generate the native platform files.

## Building On Windows

### Windows Development Environment Requirements

- Windows
  - Any Personal Computer version supported by Microsoft
- Microsoft [Visual Studio](https://www.visualstudio.com/)
  - Versions
    - [2022](https://www.visualstudio.com/vs/downloads/)
    - [2019](https://www.visualstudio.com/vs/older-downloads/)
  - The Community Edition of each of the above versions is sufficient, as
    well as any more capable edition.
- [CMake 3.22.1](https://cmake.org/files/v3.22.1/cmake-3.22.1-win64-x64.zip) is recommended.
  - Use the installer option to add CMake to the system PATH
- Git Client Support
  - [Git for Windows](http://git-scm.com/download/win) is a popular solution
    for Windows
  - Some IDEs (e.g., [Visual Studio](https://www.visualstudio.com/),
    [GitHub Desktop](https://desktop.github.com/)) have integrated
    Git client support

### Windows Build - Microsoft Visual Studio

The general approach is to run CMake to generate the Visual Studio project
files. Then either run CMake with the `--build` option to build from the
command line or use the Visual Studio IDE to open the generated solution and
work with the solution interactively.

#### Windows Quick Start

Open a developer command prompt and enter:

    cd Vulkan-Loader
    mkdir build
    cd build
    cmake -A x64 -D UPDATE_DEPS=ON ..
    cmake --build .

The above commands instruct CMake to find and use the default Visual Studio
installation to generate a Visual Studio solution and projects for the x64
architecture. The second CMake command builds the Debug (default)
configuration of the solution.

#### Use `CMake` to Create the Visual Studio Project Files

Change your current directory to the top of the cloned repository directory,
create a build directory and generate the Visual Studio project files:

    cd Vulkan-Loader
    mkdir build
    cd build
    cmake -D UPDATE_DEPS=ON -G "Visual Studio 16 2019" -A x64 ..

> Note: The `..` parameter tells `cmake` the location of the top of the
> repository. If you place your build directory someplace else, you'll need to
> specify the location of the repository differently.

The `-G` option is used to select the generator

Supported Visual Studio generators:
* `Visual Studio 17 2022`
* `Visual Studio 16 2019`

The `-A` option is used to select either the "Win32", "x64", or "ARM64 architecture.

When generating the project files, the absolute path to a Vulkan-Headers
install directory must be provided. This can be done automatically by the
`-D UPDATE_DEPS=ON` option, by directly setting the
`VULKAN_HEADERS_INSTALL_DIR` environment variable, or by setting the
`VULKAN_HEADERS_INSTALL_DIR` CMake variable with the `-D` CMake option. In
either case, the variable should point to the installation directory of a
Vulkan-Headers repository built with the install target.

The above steps create a Windows solution file named `Vulkan-Loader.sln` in
the build directory.

At this point, you can build the solution from the command line or open the
generated solution with Visual Studio.

#### Build the Solution From the Command Line

While still in the build directory:

    cmake --build .

to build the Debug configuration (the default), or:

    cmake --build . --config Release

to make a Release build.

#### Build the Solution With Visual Studio

Launch Visual Studio and open the "Vulkan-Loader.sln" solution file in the
build folder. You may select "Debug" or "Release" from the Solution
Configurations drop-down list. Start a build by selecting the Build->Build
Solution menu item.

#### Windows Install Target

The CMake project also generates an "install" target that you can use to copy
the primary build artifacts to a specific location using a "bin, include, lib"
style directory structure. This may be useful for collecting the artifacts and
providing them to another project that is dependent on them.

The default location is `$CMAKE_CURRENT_BINARY_DIR\install`, but can be changed
with the `CMAKE_INSTALL_PREFIX` variable when first generating the project build
files with CMake.

You can build the install target from the command line with:

    cmake --build . --config Release --target install

or build the `INSTALL` target from the Visual Studio solution explorer.

## Building On Linux

### Linux Development Environment Requirements

This repository has been built and tested on the two most recent Ubuntu LTS
versions, although earlier versions may work.
It is be straightforward to adapt this repository to other Linux distributions.

[CMake 3.22.1](https://cmake.org/files/v3.22.1/cmake-3.22.1-Linux-x86_64.tar.gz) is recommended.

#### Required Package List

    sudo apt-get install git build-essential libx11-xcb-dev \
        libxkbcommon-dev libwayland-dev libxrandr-dev

### Linux Build

The general approach is to run CMake to generate make files. Then either run
CMake with the `--build` option or `make` to build from the command line.

#### Linux Quick Start

    cd Vulkan-Loader
    mkdir build
    cd build
    cmake -D UPDATE_DEPS=ON ..
    make

See below for the details.

#### Use CMake to Create the Make Files

Change your current directory to the top of the cloned repository directory,
create a build directory and generate the make files.

    cd Vulkan-Loader
    mkdir build
    cd build
    cmake -D CMAKE_BUILD_TYPE=Debug \
          -D VULKAN_HEADERS_INSTALL_DIR=absolute_path_to_install_dir \
          -D CMAKE_INSTALL_PREFIX=install ..

> Note: The `..` parameter tells `cmake` the location of the top of the
> repository. If you place your `build` directory someplace else, you'll need
> to specify the location of the repository top differently.

Use `-D CMAKE_BUILD_TYPE` to specify a Debug or Release build.

When generating the project files, the absolute path to a Vulkan-Headers
install directory must be provided. This can be done automatically by the
`-D UPDATE_DEPS=ON` option, by directly setting the `VULKAN_HEADERS_INSTALL_DIR`
environment variable, or by setting the `VULKAN_HEADERS_INSTALL_DIR` CMake
variable with the `-D` CMake option.
In either case, the variable should point to the installation directory of a
Vulkan-Headers repository built with the install target.

> Note: For Linux, the default value for `CMAKE_INSTALL_PREFIX` is
> `/usr/local`, which would be used if you do not specify
> `CMAKE_INSTALL_PREFIX`. In this case, you may need to use `sudo` to install
> to system directories later when you run `make install`.

#### Build the Project

You can just run `make` to begin the build.

To speed up the build on a multi-core machine, use the `-j` option for `make`
to specify the number of cores to use for the build. For example:

    make -j4

You can also use

    cmake --build .

### Linux Notes

#### WSI Support Build Options

By default, the Vulkan Loader is built with support for the Vulkan-defined WSI
display servers: Xcb, Xlib, and Wayland. It is recommended to build the
repository components with support for these display servers to maximize their
usability across Linux platforms. If it is necessary to build these modules
without support for one of the display servers, the appropriate CMake option
of the form `BUILD_WSI_xxx_SUPPORT` can be set to `OFF`.

#### Linux Install to System Directories

Installing the files resulting from your build to the systems directories is
optional since environment variables can usually be used instead to locate the
binaries. There are also risks with interfering with binaries installed by
packages. If you are certain that you would like to install your binaries to
system directories, you can proceed with these instructions.

Assuming that you've built the code as described above and the current
directory is still `build`, you can execute:

    sudo make install

This command installs files to `/usr/local` if no `CMAKE_INSTALL_PREFIX` is
specified when creating the build files with CMake:

- `/usr/local/lib`:  Vulkan loader library and package config files

You may need to run `ldconfig` in order to refresh the system loader search
cache on some Linux systems.

You can further customize the installation location by setting additional
CMake variables to override their defaults. For example, if you would like to
install to `/tmp/build` instead of `/usr/local`, on your CMake command line
specify:

    -D CMAKE_INSTALL_PREFIX=/tmp/build

Then run `make install` as before. The install step places the files in
`/tmp/build`. This may be useful for collecting the artifacts and providing
them to another project that is dependent on them.

Using the `CMAKE_INSTALL_PREFIX` to customize the install location also
modifies the loader search paths to include searching for layers in the
specified install location. In this example, setting `CMAKE_INSTALL_PREFIX` to
`/tmp/build` causes the loader to search
`/tmp/build/etc/vulkan/explicit_layer.d` and
`/tmp/build/share/vulkan/explicit_layer.d` for the layer JSON files. The
loader also searches the "standard" system locations of
`/etc/vulkan/explicit_layer.d` and `/usr/share/vulkan/explicit_layer.d` after
searching the two locations under `/tmp/build`.

You can further customize the installation directories by using the CMake
variables `CMAKE_INSTALL_SYSCONFDIR` to rename the `etc` directory and
`CMAKE_INSTALL_DATADIR` to rename the `share` directory.

See the CMake documentation for more details on using these variables to
further customize your installation.

Also see the `LoaderInterfaceArchitecture.md` document in the `docs` folder in this
repository for more information about loader operation.

#### Linux 32-bit support

The loader supports building in 32-bit Linux environments.
However, it is not nearly as straightforward as it is for Windows.
Here are some notes for building this repo as 32-bit on a 64-bit Ubuntu
"reference" platform:

If not already installed, install the following 32-bit development libraries:

`gcc-multilib gcc-multilib g++-multilib libc6:i386 libc6-dev-i386 libgcc-s1:i386 libwayland-dev:i386 libxrandr-dev:i386`

This list may vary depending on your distribution and which windowing systems you are building for.

Set up your environment for building 32-bit targets when configuring your build:

      cmake ... -D CMAKE_CXX_FLAGS=-m32 -D CMAKE_C_FLAGS=-m32

However, you may find that pkg-config picks incorrect libraries. This is due to a CMake implementation issue:
https://gitlab.kitware.com/cmake/cmake/-/issues/25317

Your PKG_CONFIG configuration may be different, depending on your distribution.

You can the `PKG_CONFIG_PATH` environment variable to address this issue.

Finally, build the repository normally as explained above.

These notes are taken from the Github Actions workflow `linux-32` which is run regularly as a part of CI.

## Building on MacOS

### MacOS Development Environment Requirements

Setup Homebrew and components

- Ensure Homebrew is at the beginning of your PATH:

      export PATH=/usr/local/bin:$PATH

- Add packages with the following (may need refinement)

      brew install python python3 git

### Clone the Repository

Clone the Vulkan-Loader repository:

    git clone https://github.com/KhronosGroup/Vulkan-Loader.git

### MacOS build

[CMake 3.22.1](https://cmake.org/files/v3.22.1/cmake-3.22.1-Darwin-x86_64.tar.gz) is recommended.

#### Building with the Unix Makefiles Generator

This generator is the default generator.

When generating the project files, the absolute path to a Vulkan-Headers
install directory must be provided. This can be done automatically by the
`-D UPDATE_DEPS=ON` option, by directly setting the
`VULKAN_HEADERS_INSTALL_DIR` environment variable, or by setting the
`VULKAN_HEADERS_INSTALL_DIR` CMake variable with the `-D` CMake option. In
either case, the variable should point to the installation directory of a
Vulkan-Headers repository built with the install target.

    mkdir build
    cd build
    cmake -D UPDATE_DEPS=ON -D VULKAN_HEADERS_INSTALL_DIR=absolute_path_to_install_dir -D CMAKE_BUILD_TYPE=Debug ..
    make

To speed up the build on a multi-core machine, use the `-j` option for `make`
to specify the number of cores to use for the build. For example:

    make -j4

#### Building with the Xcode Generator

To create and open an Xcode project:

    mkdir build-xcode
    cd build-xcode
    cmake -GXcode ..
    open Vulkan-Loader.xcodeproj

Within Xcode, you can select Debug or Release builds in the project's Build
Settings.

## Building on Fuchsia

Fuchsia uses the project's GN build system to integrate with the Fuchsia platform build.

### SDK Symbols

The Vulkan Loader is a component of the Fuchsia SDK, so it must explicitly declare its exported symbols in
the file vulkan.symbols.api; see [SDK](https://fuchsia.dev/fuchsia-src/development/sdk).

## Building on QNX

QNX is using its own build system. The proper build environment must be set
under the QNX host development system (Linux, Win64, MacOS) by invoking
the shell/batch script provided with QNX installation.

Then change working directory to the "scripts/qnx" in this project and type "make".
It will build the ICD loader for all CPU targets supported by QNX.

## Cross Compilation

While this repo is capable of cross compilation, there are a handful of caveats.

### Unknown function handling which requires explicit assembly implementations

Unknown function handling is only fully supported on select platforms due to the
need for assembly in the implementation.
Platforms not fully supported will have assembly disabled automatically, or
can be manually disabled by setting `USE_GAS` or `USE_MASM` to `OFF`.

#### Platforms which fully support unknown function handling

* 64 bit Windows (x64)
* 32 bit Windows (x86)
* 64 bit Linux (x64)
* 32 bit Linux (x86)
* 64 bit Arm (aarch64)
* 32 bit Arm (aarch32)


Platforms not listed will use a fallback C Code path that relies on tail-call optimization to work.
No guarantees are made about the use of the fallback code paths.

## Tests

To build tests, make sure that the `BUILD_TESTS` option is set to true. Using
the command line, this looks like `-D BUILD_TESTS=ON`.

This project is configured to run with `ctest`, which makes it easy to run the
tests. To run the tests, change the directory to that of the build direction, and
execute `ctest`.

More details can be found in the [README.md](./tests/README.md) for the tests
directory of this project.
