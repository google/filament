# DirectX Shader Compiler

The DirectX Shader Compiler project includes a compiler and related tools used to compile High-Level Shader Language (HLSL) programs into DirectX Intermediate Language (DXIL) representation. Applications that make use of DirectX for graphics, games, and computation can use it to generate shader programs.

For more information, see the [Wiki](https://github.com/microsoft/DirectXShaderCompiler/wiki).

Visit the [DirectX Landing Page](https://devblogs.microsoft.com/directx/landing-page/) for more resources for DirectX developers.

## Features and Goals

The starting point of the project is a fork of the [LLVM](http://llvm.org/) and [Clang](http://clang.llvm.org/) projects, modified to accept HLSL and emit a validated container that can be consumed by GPU drivers.

At the moment, the DirectX HLSL Compiler provides the following components:

- dxc.exe, a command-line tool that can compile HLSL programs for shader model 6.0 or higher

- dxcompiler.dll, a DLL providing a componentized compiler, assembler, disassembler, and validator

- dxilconv.dll, a DLL providing a converter from DXBC (older shader bytecode format)

- dxv.exe, a command-line tool that validates DXIL IR (compiled HLSL programs). 

- various other tools based on the above components

The Microsoft Windows SDK releases include a supported version of the compiler and validator.

The goal of the project is to allow the broader community of shader developers to contribute to the language and representation of shader programs, maintaining the principles of compatibility and supportability for the platform. It's currently in active development across two axes: language evolution (with no impact to DXIL representation), and surfacing hardware capabilities (with impact to DXIL, and thus requiring coordination with GPU implementations).

### Pre-built Releases

Development kits containing only the dxc.exe driver app, the dxcompiler.dll, and the dxil.dll signing binary are available [here](https://github.com/microsoft/DirectXShaderCompiler/wiki/Releases), or in the [releases tab](https://github.com/microsoft/DirectXShaderCompiler/releases).

### SPIR-V CodeGen

As an example of community contribution, this project can also target the [SPIR-V](https://www.khronos.org/registry/spir-v/) intermediate representation. Please see the [doc](docs/SPIR-V.rst) for how HLSL features are mapped to SPIR-V, and the [wiki](https://github.com/microsoft/DirectXShaderCompiler/wiki/SPIR%E2%80%90V-CodeGen) page for how to build, use, and contribute to the SPIR-V CodeGen.

### Metal CodeGen

When built from source DXC can utilize the [Metal Shader
Converter](https://developer.apple.com/metal/shader-converter/) if it is
available during build and configuration time. This allows DXC to generate Metal
shader libraries directly using the `-metal` flag.

Note: DXC cannot currently disassemble Metal shaders so the `-Fc` flag cannot be
used in conjunction with the `-Fo` flag.

## Building Sources

See the full documentation for [Building and testing DXC](docs/BuildingAndTestingDXC.rst) for detailed instructions.

## Running Shaders

To run shaders compiled as DXIL, you will need support from the operating system as well as from the driver for your graphics adapter. Windows 10 Creators Update is the first version to support DXIL shaders. See the [Wiki](https://github.com/microsoft/DirectXShaderCompiler/wiki/Running-Shaders) for information on using experimental support or the software adapter.

### Hardware Support

Hardware GPU support for DXIL is provided by the following vendors:

#### NVIDIA
NVIDIA's r396 drivers (r397.64 and later) provide release mode support for DXIL
1.1 and Shader Model 6.1 on Win10 1709 and later, and experimental mode support
for DXIL 1.2 and Shader Model 6.2 on Win10 1803 and later. These drivers also
support DXR in experimental mode.

Drivers can be downloaded from [geforce.com](https://www.geforce.com/drivers).

#### AMD
AMD’s driver (Radeon Software Adrenalin Edition 18.4.1 or later) provides release mode support for DXIL 1.1 and Shader Model 6.1. Drivers can be downloaded from [AMD's download site](https://support.amd.com/en-us/download).

### Intel
Intel's 15.60 drivers (15.60.0.4849 and later) support release mode for DXIL 1.0 and Shader Model 6.0 as well as
release mode for DXIL 1.1 and Shader Model 6.1 (View Instancing support only).

Drivers can be downloaded from the following link [Intel Graphics Drivers](https://downloadcenter.intel.com/product/80939/Graphics-Drivers)

Direct access to 15.60 driver (latest as of this update) is provided below:

[Installer](https://downloadmirror.intel.com/27412/a08/win64_15.60.2.4901.exe)

[Release Notes related to DXIL](https://downloadmirror.intel.com/27266/eng/ReleaseNotes_GFX_15600.4849.pdf)

## Making Changes

To make contributions, see the [CONTRIBUTING.md](CONTRIBUTING.md) file in this project.

## Documentation

You can find documentation for this project in the `docs` directory. These contain the original LLVM documentation files, as well as two new files worth nothing:

* [HLSLChanges.rst](docs/HLSLChanges.rst): this is the starting point for how this fork diverges from the original llvm/clang sources
* [DXIL.rst](docs/DXIL.rst): this file contains the specification for the DXIL format
* [tools/clang/docs/UsingDxc.rst](tools/clang/docs/UsingDxc.rst): this file contains a user guide for dxc.exe

## License

DirectX Shader Compiler is distributed under the terms of the University of Illinois Open Source License.

See [LICENSE.txt](LICENSE.TXT) and [ThirdPartyNotices.txt](ThirdPartyNotices.txt) for details.

## Code of Conduct

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/). For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.
