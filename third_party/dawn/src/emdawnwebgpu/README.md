# "emdawnwebgpu" (Dawn's fork of Emscripten's WebGPU bindings)

"emdawnwebgpu" is Dawn's fork of the Emscripten WebGPU bindings
(`library_webgpu.js` and friends). The forked files live in
[`//third_party/emdawnwebgpu`](../third_party/emdawnwebgpu/)
and the build targets in this directory produce the other files needed to build
an Emscripten-based project using these bindings.

We keep the `webgpu.h` interface roughly in sync between Dawn and emdawnwebgpu,
however we don't guarantee it will always be in sync - we don't have any
automated testing for this, so we'll periodically fix
it up as needed for import into other projects that use these bindings.

Projects should use this fork (by compiling Dawn as instructed below) if they
want the latest version, which is mostly compatible with the same version of Dawn
Native. For the future of this fork, please see <https://crbug.com/371024051>.

## Setting up Emscripten

- Get an emsdk toolchain (at least Emscripten 4.0.3, which includes the necessary tools in the
  package release). There are two options to do this:
  - Set the `dawn_wasm` gclient variable (use
    [`standalone-with-wasm.gclient`](../../scripts/standalone-with-wasm.gclient)
    as your `.gclient`), and `gclient sync`. This installs emsdk in `//third_party/emsdk`.

  - Install it manually following the official
  [instructions](https://emscripten.org/docs/getting_started/downloads.html#installation-instructions-using-the-emsdk-recommended).

## Building the bindings

First, get the Dawn code and its dependencies.
See [building.md](../../docs/building.md).

You can either build the bindings "standalone" and link them manually,
or if your project uses CMake you can link to it as a CMake subproject.

### Using Dawn as a CMake subproject (bindings only)

TODO(crbug.com/371024051): Provide a sample!

### Using pre-built emdawnwebgpu bindings via CMake (bindings only)

TODO(crbug.com/371024051): Make pre-built bindings and provide a sample!

### Standalone with CMake (bindings and samples)

Set up the build directory using emcmake:

```
mkdir out/cmake-wasm
cd out/cmake-wasm

path/to/emsdk/upstream/emscripten/emcmake cmake ../..

make -j8
```

(To use Ninja instead of Make, for better parallelism, add `-GNinja` to the
`cmake` invocation, and build using `ninja`.)

The resulting html files can then be served and viewed in a compatible browser.

### Standalone with GN (bindings only)

- Set up a Dawn GN build, with `dawn_emscripten_dir` in the GN args set to point to
  `emsdk/upstream/emscripten`.

- Build the `emdawnwebgpu` GN build target.

- Configure the Emscripten build with all of the linker flags listed in `emdawnwebgpu_config`
  (and without Emscripten's `-sUSE_WEBGPU` setting, because we don't want the built-in bindings).
