# Building Emdawnwebgpu

**Emdawnwebgpu is easiest to use as a pre-built package from
<https://github.com/google/dawn/releases>. For information on using those,
and other general information, read [`pkg/README.md`](pkg/README.md) instead.**

This README discusses building the pre-packaged `emdawnwebgpu_pkg`, building the
in-tree Dawn samples/tests for Wasm, and using our CMake or GN build files to
link either Dawn (for native) or Emdawnwebgpu (for Wasm).

## Setting up a CMake project that automatically chooses Dawn or Emdawnwebgpu

Please read <https://developer.chrome.com/docs/web-platform/webgpu/build-app>.

## Building emdawnwebgpu and emdawnwebgpu_pkg

First, get the Dawn code and its dependencies.
See [building.md](../../docs/building.md).

To build the package, you'll build Dawn's `emdawnwebgpu_pkg` target using
Emscripten. `out/yourbuild/emdawnwebgpu_pkg` combines files from:
- `src/emdawnwebgpu`
- `third_party/emdawnwebgpu`
- `out/yourbuild/gen`

### Set up Emscripten

Get an emsdk toolchain (at least Emscripten 4.0.3, which includes the necessary
tools in the package release). There are two options to do this:

- Set the `dawn_wasm` gclient variable (use
  [`standalone-with-wasm.gclient`](../../scripts/standalone-with-wasm.gclient)
  as your `.gclient`), and `gclient sync`.
  This installs emsdk in `//third_party/emsdk`.
- Install it manually following the official
  [instructions](https://emscripten.org/docs/getting_started/downloads.html#installation-instructions-using-the-emsdk-recommended).

### Standalone with CMake

Set up the build directory using emcmake:

```sh
mkdir out/cmake-wasm
cd out/cmake-wasm

path/to/emsdk/upstream/emscripten/emcmake cmake ../..

# Package
make -j8 emdawnwebgpu_pkg
```

Samples and tests:

```sh
# Samples (for a list of samples, see ENABLE_EMSCRIPTEN targets in src/dawn/samples/CMakeLists.txt)
make -j8 HelloTriangle

# Tests
make -j8 emdawnwebgpu_tests_asyncify emdawnwebgpu_tests_jspi
```

(To use Ninja instead of Make, for better parallelism, add `-GNinja` to the
`cmake` invocation, and build using `ninja`.)

Samples and tests produce HTML files which can be served and viewed in a compatible browser.

### Standalone with GN

- Set up Emscripten as per instructions above using `dawn_wasm`.

- Build the `emdawnwebgpu` and `samples` GN build targets.

Samples and tests produce HTML files in `out/<dir>/wasm` which can be served and viewed in a compatible browser.

## Appendix: Bits of Emscripten we depend on

(Internal docs for coordination between Emscripten and Emdawnwebgpu.)

Emdawnwebgpu depends on a lot of random bits of Emscripten that aren't usually
public. We assume they're "mostly stable" for now and will update this (and roll
into Emscripten) as needed if they change. Most, but not all, incompatible
changes will be detected on Emscripten's CI, because it tests Emdawnwebgpu. This
list is a best effort to enumerate them, but definitely isn't complete.

- Remote ports
- In `emdawnwebgpu.port.py`:
  - Ports API in general
  - Modifying various settings in `linker_setup()`
  - `tools.diagnostics.error`
- In package build:
  - `tools/gen_struct_info.py`
- In `library_webgpu.js`:
  - Various items from `parseTools` and `jsifier`
    - `*__i53abi: true`
    - `makeGetValue`
    - `runtimeKeepalivePush`/`runtimeKeepalivePop`
  - Public settings:
    - `ASSERTIONS`
    - `ASYNCIFY`
    - `USE_WEBGPU`
    - `MEMORY64`, `WASM_BIGINT`
  - `settings_internal.js` settings:
    - `CAN_ADDRESS_2GB`
  - Public preamble:
    - `assert`, `abort`
  - Internal library functions:
    - `stackSave`, `stackRestore`
    - `stringToUTF8OnStack`, `UTF8ToString`, `stringToNewUTF8`, `lengthBytesUTF8`
    - `callUserCallback`
    - `warnOnce`
    - `readI53FromI64`/`writeI53ToI64`
    - `findCanvasEventTarget`
- In `webgpu.cpp`:
  - Public APIs:
    - `emscripten_has_asyncify`
