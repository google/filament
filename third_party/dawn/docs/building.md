# Building Dawn

## System requirements

 * Git
 * CMake (3.10.2 or later) (if desired)
 * GN (if desired)
 * Ninja (or other build tool)
 * Python, for fetching dependencies
 * [depot_tools] in your path

- Linux
  - The `pkg-config` command:
    ```sh
    # Install pkg-config on Ubuntu
    sudo apt-get install pkg-config
    ```

- Mac
  - [Xcode](https://developer.apple.com/xcode/) 12.2+.
  - The macOS 11.0 SDK. Run `xcode-select` to check whether you have it.
    ```sh
    ls `xcode-select -p`/Platforms/MacOSX.platform/Developer/SDKs
    ```

## Get the code and its dependencies

### Using `depot_tools`

Dawn uses the Chromium build system and dependency management so you need to [install depot_tools] and add it to the PATH.

[install depot_tools]: http://commondatastorage.googleapis.com/chrome-infra-docs/flat/depot_tools/docs/html/depot_tools_tutorial.html#_setting_up

```sh
# Clone the repo as "dawn"
git clone https://dawn.googlesource.com/dawn dawn && cd dawn

# Bootstrap the gclient configuration
cp scripts/standalone.gclient .gclient

# Fetch external dependencies and toolchains with gclient
gclient sync
```

### Without `depot_tools`

If you cannot or do not want to depend on `depot_tools`, you may use the `tools/fetch_dawn_dependencies.py` to clone the dependencies' repositories:

```sh
# Clone the repo as "dawn"
git clone https://dawn.googlesource.com/dawn dawn && cd dawn

# Fetch dependencies (lose equivalent of gclient sync)
python tools/fetch_dawn_dependencies.py --use-test-deps
```

Use `python tools/fetch_dawn_dependencies.py -h` to know more about the available options. The `--use-test-deps` option used above specifies to also fetch dependencies needed by tests. Contrary to `depot_tools`, this scripts does not figure out option-dependent requirements automatically.

### Linux dependencies

The following packages are needed to build Dawn. (Package names are the Ubuntu names).

* `libxrandr-dev`
* `libxinerama-dev`
* `libxcursor-dev`
* `mesa-common-dev`
* `libx11-xcb-dev`
* `pkg-config`
* `nodejs`
* `npm`

```sh
sudo apt-get install libxrandr-dev libxinerama-dev libxcursor-dev mesa-common-dev libx11-xcb-dev pkg-config nodejs npm
```

Note, `nodejs` and `npm` are only needed if building `dawn.node`.


## Build Dawn

### Compiling using CMake + Ninja
```sh
mkdir -p out/Debug
cd out/Debug
cmake -GNinja ../..
ninja # or autoninja
```

### Compiling using CMake + make
```sh
mkdir -p out/Debug
cd out/Debug
cmake ../..
make # -j N for N-way parallel build
```

### Compiling using gn + ninja
```sh
mkdir -p out/Debug
gn gen out/Debug
autoninja -C out/Debug
```

The most common GN build option is `is_debug=true/false`; otherwise
`gn args out/Debug --list` shows all the possible options.

On macOS you'll want to add the `use_system_xcode=true` in most cases.
(and if you're a googler please get XCode from go/xcode).

To generate a Microsoft Visual Studio solution, add `ide=vs2022` and
`ninja-executable=<dawn parent directory>\dawn\third_party\ninja\ninja.exe`.
The .sln file will be created in the output directory specified.

For CMake builds, you can enable installation with the `DAWN_ENABLE_INSTALL` option
and use `find_package(Dawn)` in your CMake project to discover Dawn and link with
the `dawn::webgpu_dawn` target. Please see [Quickstart with CMake](./quickstart-cmake.md)
for step-by-step instructions.

### Fuzzers on MacOS
If you are attempting fuzz, using `TINT_BUILD_FUZZERS=ON`, the version of llvm
in the XCode SDK does not have the needed libfuzzer functionality included.

The build error that you will see from using the XCode SDK will look something
like this:
```
ld: file not found:/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/lib/clang/11.0.0/lib/darwin/libclang_rt.fuzzer_osx.a
```

The solution to this problem is to use a full version llvm, like what you would
get via homebrew, `brew install llvm`, and use something like `CC=<path to full
clang> cmake ..` to setup a build using that toolchain.

