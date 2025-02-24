# DirectX Shader Compiler Redistributable Package

This package contains a copy of the DirectX Shader Compiler redistributable and its associated development headers.

For help getting started, please see:

<https://github.com/microsoft/DirectXShaderCompiler/wiki>

## Licenses

The included licenses apply to the following files:

| License file | Applies to |
|---|---|
|LICENSE-MIT.txt    |d3d12shader.h|
|LICENSE-LLVM.txt   |all other files|

## Changelog

### Upcoming Release

Place release notes for the upcoming release below this line and remove this line upon naming this release.

### Version 1.8.2502

- The incomplete WaveMatrix implementation has been removed. [#6807](https://github.com/microsoft/DirectXShaderCompiler/pull/6807)
- DXIL Validator Hash is open sourced. [#6846](https://github.com/microsoft/DirectXShaderCompiler/pull/6846)
- DXIL container validation for PSV0 part allows any content ordering inside string and semantic index tables. [#6859](https://github.com/microsoft/DirectXShaderCompiler/pull/6859)
- The and() and or() intrinsics will now accept non-integer parameters by casting them to bools. [#7060](https://github.com/microsoft/DirectXShaderCompiler/pull/7060)

### Version 1.8.2407

This cumulative release contains numerous bug fixes and stability improvments.

Here are some highlights:

- dxc generates invalid alignment on groupshared matrix load/store instructions in [#6416](https://github.com/microsoft/DirectXShaderCompiler/issues/6416)
- [Optimization] DXC is missing common factor optimization in some cases in [#6593](https://github.com/microsoft/DirectXShaderCompiler/issues/6593)
- [SPIR-V] Implement WaveMutliPrefix* in [#6600](https://github.com/microsoft/DirectXShaderCompiler/issues/6600)
- [SPIR-V] Implement SampleCmpLevel for SM6.7 in [#6613](https://github.com/microsoft/DirectXShaderCompiler/issues/6613)
- Avoid adding types to default namespace in [#6646](https://github.com/microsoft/DirectXShaderCompiler/issues/6646)
- Release notes once found in `README.md` can now be found in `ReleaseNotes.md`
- Fixed several bugs in the loop restructurizer. Shader developers who are using -opt-disable structurize-loop-exits-for-unroll to disable the loop restructurizer should consider removing that workaround.

### Version 1.8.2405

DX Compiler Release for May 2024

This release includes two major new elements:

- The introduction of the first component of HLSL 202x
- The inclusion of clang-built Windows binaries

See [the official blog post](https://devblogs.microsoft.com/directx/dxc-1-8-2405-available) for a more detailed description of this release.

HLSL 202x is a placeholder designation for what will ultimately be a new language version that further aligns HLSL with modern language features. It is intended to serve as a bridge to help transition to the expected behavior of the modernized compiler.

To experiment with 202x, use the `-HV 202x` flag. We recommend enabling these warnings as well to catch potential changes in behavior: `-Wconversion -Wdouble-promotion -Whlsl-legacy-literal`.

The first feature available in 202x updates HLSL's treatment of literals to better conform with C/C++. In previous versions, un-suffixed literal types targeted the highest possible precision. This feature revises that to mostly conform with C/C++ behavior. See the above blog post for details.

Clang-built Windows binaries are included in addition to the MSVC-built binaries that have always been shipped before. The clang-built compiler is expected to improve HLSL compile times in many cases. We are eager for feedback about this build positive or negative, related to compile times or correctness.

### Version 1.8.2403.2

DX Compiler Release for March 2024 - Patch 2

- Fix regression: [#6426](https://github.com/microsoft/DirectXShaderCompiler/issues/6426) Regression, SIGSEGV instead of diagnostics when encountering bool operator==(const T&, const T&).

### Version 1.8.2403.1

DX Compiler Release for March 2024 - Patch 1

- Fix regression: [#6419](https://github.com/microsoft/DirectXShaderCompiler/issues/6419) crash when using literal arguments with `fmod`.

### Version 1.8.2403

DX Compiler release for March 2024

- Shader Model 6.8 is fully supported
  - Work Graphs allow node shaders with user-defined input and output payloads
  - New Barrier builtin functions with specific memory types and semantics
  - Expanded Comparison sampler intrinsics: SampleCmpBias, SampleCmpGrad, and CalculateLevelOfDetail
  - StartVertexLocation and StartInstanceLocation semantics
  - WaveSizeRange entry point attribute allows specifying a range of supported wave sizes
- Improved compile-time validation and runtime validation information
- Various stability improvements including numerous address sanitation fixes
- Several Diagnostic improvements
  - Many diagnostics are generated earlier and with more detailed information
  - Library profile diagnostic improvements
  - No longer infer library shader type when not specified
  - More helpful diagnostics for numthreads and other entry point attributes
  - Validation errors more accurately determine usage by the entry point
- Improve debug info generation
- Further improvements to Linux build quality
- File paths arguments for `IDxcIncludeHandler::LoadSource` will now be normalized to use OS specific slashes
  (`\` for windows, `/` for *nix) and no longer have double slashes except for UNC paths (`\\my\unc\path`).”

### Version 1.7.2308

DX Compiler release for August 2023

- HLSL 2021 is now enabled by default
- Various HLSL 2021 fixes have been made to
  - Operator overloading fixes
  - Templates fixes
  - Select() with samplers
  - Bitfields show in reflections
  - Bitfields can be used on enums
  - Allow function template default params
- Issues with loading and using Linux binaries have been resolved
- Support #pragma region/endregion
- Various stability and diagnostic improvements
- Dxcapi.h inline documentation is improved
- Linking of libraries created by different compilers is disallowed to prevent interface Issues
- Inout parameter correctness improved

The package includes dxc.exe, dxcompiler.dll, corresponding lib and headers, and dxil.dll for x64 and arm64 platforms on Windows.
The package also includes Linux version of the compiler with corresponding executable, libdxcompiler.so, corresponding headers, and libdxil.so for x64 platforms.

The new DirectX 12 Agility SDK (Microsoft.Direct3D.D3D12 nuget package) and a hardware driver with appropriate support
are required to run shader model 6.7 shaders. Please see <https://aka.ms/directx12agility> for details.

The SPIR-V backend of the compiler has been enabled in this release.

### Version 1.7.2212

DX Compiler release for December 2022.

- Includes full support of HLSL 2021 for SPIRV generation as well as many HLSL 2021 fixes and enhancements:
  - HLSL 2021's `and`, `or` and `select` intrinsics are now exposed in all language modes. This was done to ease porting code bases to HLSL2021, but may cause name conflicts in existing code.
  - Improved template utility with user-defined types
  - Many additional bug fixes
- Linux binaries are now included.
 This includes the compiler executable, the dynamic library, and the dxil signing library.
- New flags for inspecting compile times:
  - `-ftime-report` flag prints a high level summary of compile time broken down by major phase or pass in the compiler. The DXC
command line will print the output to stdout.
  - `-ftime-trace` flag prints a Chrome trace json file. The output can be routed to a specific file by providing a filename to
the argument using the format `-ftime-trace=<filename>`. Chrome trace files can be opened in Chrome by loading the built-in tracing tool
at chrome://tracing. The trace file captures hierarchial timing data with additional context enabling a much more in-depth profiling
experience.
  - Both new options are supported via the DXC API using the `DXC_OUT_TIME_REPORT` and `DXC_OUT_TIME_TRACE` output kinds respectively.
- IDxcPdbUtils2 enables reading new PDB container part
- `-P` flag will now behave as it does with cl using the file specified by `-Fi` or a default
- Unbound multidimensional resource arrays are allowed
- Diagnostic improvements
- Reflection support on non-Windows platforms; minor updates adding RequiredFeatureFlags to library function reflection and thread group size for AS and MS.

The package includes dxc.exe, dxcompiler.dll, corresponding lib and headers, and dxil.dll for x64 and arm64 platforms on Windows.
For the first time the package also includes Linux version of the compiler with corresponding executable, libdxcompiler.so, corresponding headers, and libdxil.so for x64 platforms.

The new DirectX 12 Agility SDK (Microsoft.Direct3D.D3D12 nuget package) and a hardware driver with appropriate support
are required to run shader model 6.7 shaders. Please see <https://aka.ms/directx12agility> for details.

The SPIR-V backend of the compiler has been enabled in this release. Please note that Microsoft does not perform testing/verification of the SPIR-V backend.

### Version 1.7.2207

DX Compiler release for July 2022. Contains shader model 6.7 and many bug fixes and improvements, such as:

- Features: Shader Model 6.7 includes support for Raw Gather, Programmable Offsets, QuadAny/QuadAll, WaveOpsIncludeHelperLanes, and more!
- Platforms: ARM64 support
- HLSL 2021 : Enable “using” keyword
- Optimizations: Loop unrolling and dead code elimination improvements
- Developer tools: Improved disassembly output

The package includes dxc.exe, dxcompiler.dll, corresponding lib and headers, and dxil.dll for x64 and, for the first time, arm64 platforms!

The new DirectX 12 Agility SDK (Microsoft.Direct3D.D3D12 nuget package) and a hardware driver with appropriate support
are required to run shader model 6.7 shaders. Please see <https://aka.ms/directx12agility> for details.

The SPIR-V backend of the compiler has been enabled in this release. Please note that Microsoft does not perform testing/verification of the SPIR-V backend.
