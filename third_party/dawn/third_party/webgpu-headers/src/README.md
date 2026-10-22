# WebGPU Headers

This repository contains the stable C header `webgpu.h`, a C API equivalent of
the [WebGPU](https://gpuweb.github.io/gpuweb/) API for JavaScript.

All of the C API is defined in the [`webgpu.h`](./webgpu.h) header file.
This header, its documentation, and the attached documentation articles
describe the native specifities of the C API;
for everything else, refer to the WebGPU specification.
Together, these form a rough "specification" for `webgpu.h`.

Multiple implementations strive to implement this "specification" compatibly,
in addition to their own extensions which are not specified here.
For access to those extensions, use your implementation's extended headers.

### [Read the documentation here!](https://webgpu-native.github.io/webgpu-headers/)

## Why?

While WebGPU is a JavaScript API made for the Web, it is a good tradeoff of ergonomic, efficient and portable graphics API.
Almost all of its concepts are not specific to the Web platform and the headers replicate them exactly, while adding capabilities to interact with native concepts (like windowing).

Implementations of this header include:

 - [Dawn](https://dawn.googlesource.com/dawn), the C++ WebGPU implementation used in Chromium
 - [Emdawnwebgpu](https://dawn.googlesource.com/dawn/+/refs/heads/main/src/emdawnwebgpu/pkg/README.md) for Emscripten translates `webgpu.h` calls to JavaScript WebGPU calls when compiling to WebAssembly.
 - [wgpu-native](https://github.com/gfx-rs/wgpu-native), C bindings to [wgpu](https://github.com/gfx-rs/wgpu), the Rust WebGPU implementation used in Firefox.
    - **wgpu-native does not yet implement the stable version of this header. Contributions needed!**

## `webgpu.yml` and `webgpu.json`

`webgpu.yml` and `webgpu.json` (same contents, different formats) are the main machine-readable source of truth for the C API and its documentation. This data is used to generate the official `webgpu.h` header present in this repository to generate the official documentation, and may be used by any other third party to design tools and wrappers around WebGPU-Native, especially bindings into other languages.

**If you are developing bindings of `webgpu.h` into another language and find that any additional high-level/semantic information would be useful in `webgpu.yml`, please contribute it!**

## Contributing to this project

**Important:** When submitting a change, one must modify `webgpu.yml`, `webgpu.json`, and `webgpu.h` files in a consistent way. One should first edit `webgpu.yml` (the source of truth), then run `make gen` to update `webgpu.json` and `webgpu.h`, and finally commit all changes together.

Here are some details about the structure of this repository.

### Main files

 - `webgpu.h` is the one and only header file that defines the WebGPU C API. Only this needs to be integrated in a C project that links against a WebGPU implementation. (But be sure to use the headers from your implementation if you need any extensions, or if the implementation doesn't match this header exactly.)

 - `webgpu.yml` and `webgpu.json` - see above.

 - `schema.json` is the [JSON schema](https://json-schema.org/) that formally specifies the structure of `webgpu.yml`.

### Generator

 - `Makefile` defines the rules to automatically generate `webgpu.h` from `webgpu.yml` and check the result.

 - `gen/` and the `go.*` files are the source code of the generator called by the `Makefile`.

 - `tests/compile` is used to check that the generated C header is indeed valid C/C++ code.

### Workflows

 - `.github/workflows` defines the automated processes that run upon new commits/PR, to check that changes in `webgpu.yml` and `webgpu.h` are consistent.
