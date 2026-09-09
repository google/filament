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

Place release notes for the upcoming release below this line and remove this
line upon naming the release. Refer to previous for appropriate section names.

#### Bug Fixes

- Fixed derivative operations being moved into divergent control flow, which
  could produce incorrect results
  [#8001](https://github.com/microsoft/DirectXShaderCompiler/issues/8001).
- SPIR-V: Fixed an invalid `OpSelect` being generated when optimizing for
  SPIR-V 1.3 and earlier
  [#8603](https://github.com/microsoft/DirectXShaderCompiler/issues/8603).
- Fix a crash generating DXIL from sources containing a dynamic resource heap
  access that was discarded. Identified during development of SPIR-V support for
  [descriptor heaps](https://github.com/microsoft/DirectXShaderCompiler/pull/8517#discussion_r3752113078).

#### HLSL Language

- Casting a scalar to a struct or array containing a resource is now an error
  instead of crashing
  [#6661](https://github.com/microsoft/DirectXShaderCompiler/issues/6661).

#### Bug Fixes

- Fixed internal compiler errors when a member method is called on a ray payload
  or on one of its fields with payload access qualifiers enabled
  [#6464](https://github.com/microsoft/DirectXShaderCompiler/issues/6464).

### Upcoming Preview Release

These changes apply to experimental preview shader models only and will not be
part of the next non-preview release.

#### Experimental Shader Model 6.10

These are incremental changes to the experimental Shader Model 6.10 features that
first shipped in the 1.10.2605 preview.

- Fixed the set of numeric types allowed in LinAlg matrix intrinsics
  [#8271](https://github.com/microsoft/DirectXShaderCompiler/issues/8271).
- Corrected the parameter order of `InterlockedAccumulate`
  [#8459](https://github.com/microsoft/DirectXShaderCompiler/pull/8459).
- Added validation of LinAlg matrix builtin parameters and result K dimension
  [#8588](https://github.com/microsoft/DirectXShaderCompiler/pull/8588).
- Restricted the component types allowed in LinAlg matrices
  [#8608](https://github.com/microsoft/DirectXShaderCompiler/pull/8608).
- Added `BFloat16` to the ComponentType enum in DxilConstants and the linalg
  header [#8722](https://github.com/microsoft/DirectXShaderCompiler/issues/8722)

### Version 1.9.2607

#### HLSL Language

- Added `auto` for C++11-style type deduction
  [#8452](https://github.com/microsoft/DirectXShaderCompiler/pull/8452). `auto`
  cannot be used to infer built-in internal types or the types returned by
  `ResourceDescriptorHeap`/`SamplerDescriptorHeap`.
- Added the `[[nodiscard]]` attribute
  [#8462](https://github.com/microsoft/DirectXShaderCompiler/pull/8462).
- Added bit-precise floating-point literal suffixes
  [#8478](https://github.com/microsoft/DirectXShaderCompiler/pull/8478).
- `>>` no longer requires surrounding spaces in nested template arguments
  [#8453](https://github.com/microsoft/DirectXShaderCompiler/pull/8453).
- The `volatile` keyword is now disallowed
  [#8391](https://github.com/microsoft/DirectXShaderCompiler/issues/8391).
- The `groupshared` parameter attribute can now be used on templates
  [#8217](https://github.com/microsoft/DirectXShaderCompiler/pull/8217).
- Added `GroupSharedLimit` attribute support for mesh, amplification, and node
  shaders [#8140](https://github.com/microsoft/DirectXShaderCompiler/pull/8140).

#### SPIR-V

- Added sampler and resource heaps for textures
  [#8281](https://github.com/microsoft/DirectXShaderCompiler/pull/8281).
- Function parameters can now be decorated with inline SPIR-V
  [#8103](https://github.com/microsoft/DirectXShaderCompiler/issues/8103).
- Fixed `vk::BufferPointer` cast methods
  [#8365](https://github.com/microsoft/DirectXShaderCompiler/pull/8365).
- Fixed layout-rule propagation for `ConstantBuffer`/`TextureBuffer` function
  variable and parameter aliasing
  [#8244](https://github.com/microsoft/DirectXShaderCompiler/issues/8244).
- Fixed counter handling in a direct `return` statement
  [#8215](https://github.com/microsoft/DirectXShaderCompiler/issues/8215).
- Fixed `OpSpecConstant` for composites
  [#8278](https://github.com/microsoft/DirectXShaderCompiler/pull/8278).
- Fixed a crash with out-of-line template declarations
  [#5823](https://github.com/microsoft/DirectXShaderCompiler/issues/5823).
- Fixed handling of `void` in extended instruction sets
  [#8012](https://github.com/microsoft/DirectXShaderCompiler/issues/8012).

#### Bug Fixes

- Fixed GVN/SROA miscompilation of minimum-precision vector element access
  [#8268](https://github.com/microsoft/DirectXShaderCompiler/issues/8268).
- Fixed `rawBufferVectorLoad`/`Store` to widen minimum-precision types to 32-bit
  [#8273](https://github.com/microsoft/DirectXShaderCompiler/issues/8273).
- Fixed out-of-bounds subscript indexing of a column-major matrix in a constant
  buffer [#7865](https://github.com/microsoft/DirectXShaderCompiler/issues/7865).
- Fixed illegal-width bitmap generated from a `switch` lookup table in
  SimplifyCFG
  [#8421](https://github.com/microsoft/DirectXShaderCompiler/issues/8421).
- Fixed a crash producing diagnostics for source containing embedded nulls
  [#8164](https://github.com/microsoft/DirectXShaderCompiler/pull/8164).
- Fixed constant folding of `VectorReduce.*`
  [#8570](https://github.com/microsoft/DirectXShaderCompiler/issues/8570).
- GVN no longer coerces a vector store through `i128` or wider
  [#8573](https://github.com/microsoft/DirectXShaderCompiler/issues/8573).
- Validation now drills through chained GEPs when resolving TGSM globals, needed
  for SM 6.9 native vectors
  [#8571](https://github.com/microsoft/DirectXShaderCompiler/issues/8571).
- The sampler feedback shader flag is now set when sampler feedback operations
  are used and the validator version is >= 1.9
  [#8533](https://github.com/microsoft/DirectXShaderCompiler/issues/8533).
- Preserve coherence qualifiers (`globallycoherent`/`reordercoherent`) on
  resource flat-conversions
  [#8583](https://github.com/microsoft/DirectXShaderCompiler/pull/8583).
- Fixed type annotation serialization for resources in HL modules
  [#8440](https://github.com/microsoft/DirectXShaderCompiler/issues/8440).
- Fixed the `COMPARISON_NONE` handling and its error message
  [#8246](https://github.com/microsoft/DirectXShaderCompiler/issues/8246).
- Fixed an ambiguous overloaded `operator+` error with newer Clang
  [#8516](https://github.com/microsoft/DirectXShaderCompiler/pull/8516).
- Stopped emitting illegal `*.with.overflow` intrinsics for DXIL, which caused
  validation failures for overflow-check idioms when optimizations were enabled
  [#8600](https://github.com/microsoft/DirectXShaderCompiler/pull/8600).

#### Other Changes

- Built-in HLSL headers are now embedded in the dxcompiler library so that users do not need to copy headers around with the toolchain.
- vector_utils.h and enable_if.h are renamed to vector_utils and enable_if respectively in alignment with TC57 decision on standard header files to exclude file extensions (See: https://github.com/hlsl-tc57/tc57/blob/main/docs/DesignConsiderations.md#minor-details).
- Added `-Fre` support for the Metal backend to emit the Metal Shader Converter
  reflection JSON
  [#8159](https://github.com/microsoft/DirectXShaderCompiler/pull/8159).

### Version 1.10.2605

#### Experimental Shader Model 6.10

- Removed experimental Cooperative Vector, this has been replaced by LinAlg matrix.
- Implement GetGroupWaveIndex and GetGroupWaveCount in experimental Shader Model 6.10.
  - [proposal](https://github.com/microsoft/hlsl-specs/blob/main/proposals/0048-group-wave-index.md)
  - GetGroupWaveIndex: New intrinsic for Compute, Mesh, Amplification and Node shaders which returns the index of the wave within the thread group that the the thread is executing.
  - GetGroupWaveCount: New intrinsic for Compute, Mesh, Amplification and Node
  shaders which returns the total number of waves executing within the thread
  group.
- Added `DebugBreak()` and `dx::IsDebuggingEnabled()` intrinsics for shader debugging (experimental Shader Model 6.10).
  - `DebugBreak()` triggers a breakpoint if debugging is enabled.
  - `dx::IsDebuggingEnabled()` returns true if debugging is enabled when the
    intrinsic executes.
  - SPIR-V: `DebugBreak()` emits `NonSemantic.DebugBreak` extended instruction; `IsDebuggingEnabled()` is not supported.

#### Bug Fixes

- Fixed non-deterministic DXIL/PDB output when compiling shaders with resource
  arrays, debug info, and SM 6.6+.
  [#8171](https://github.com/microsoft/DirectXShaderCompiler/issues/8171)
- Fixed mesh shader semantics that were incorrectly case sensitive.
- User-defined conversion operators (e.g., `operator float4()`) now produce an
  error instead of being silently ignored.
  [#5103](https://github.com/microsoft/DirectXShaderCompiler/pull/8206)
- DXIL validation: added validation for `CreateHandleFromBinding`.
- DXIL validation now rejects non-standard integer bit widths (e.g. `i25`) in
  instructions.

#### Other Changes

- `/P` now matches `cl.exe` behavior: preprocesses to `<inputname>.i` by
  default, with `/Fi` to override the output filename. The old FXC-style `/P
   <filename>` positional syntax has been renamed to `/Po`.
  [#4611](https://github.com/microsoft/DirectXShaderCompiler/issues/4611).
- SPIR-V: Support `vk::SampledTexture` types (GLSL's `samplerND` equivalent)
  [#7979](https://github.com/microsoft/DirectXShaderCompiler/issues/7979). With
  this type, users no longer need to define both Sampler and Texture resources
  with the same binding number.

### Version 1.9.2602

#### Shader Model 6.9 Release

- Shader Model 6.9 is fully supported.
  - See [the official blog
  post](https://devblogs.microsoft.com/directx/shader-model-6-9-dxr-1-2-and-agilitysdk-1-619-release)
  for more details.

#### Noteble SPIR-V updates

- Handle vector element assignment for asuint. [#8011](https://github.com/microsoft/DirectXShaderCompiler/issues/8011)
- Support sizeof(vk::BufferPointer). [#8010](https://github.com/microsoft/DirectXShaderCompiler/issues/8010)
- Scalar layout to follow C structure layout. [#7996](https://github.com/microsoft/DirectXShaderCompiler/issues/7996)
- Support asdouble() for uint3 argument type. [#7965](https://github.com/microsoft/DirectXShaderCompiler/issues/7965)
- Add const to many FeatureManager fns. [#7980](https://github.com/microsoft/DirectXShaderCompiler/issues/7980)
- Handle vk::BufferPointer in initializer list. [#7946](https://github.com/microsoft/DirectXShaderCompiler/issues/7946)
- Fix layout rule on ConstantBuffer alias. [#7960](https://github.com/microsoft/DirectXShaderCompiler/issues/7960)
- Fix layout rule for BufferPointer pointee type. [#7956](https://github.com/microsoft/DirectXShaderCompiler/issues/7956)
- Use desugared type when processing binary op. [#7948](https://github.com/microsoft/DirectXShaderCompiler/issues/7948)
- Handle associated counters for RWStructuredBuffer in base classes. [#7880](https://github.com/microsoft/DirectXShaderCompiler/issues/7880)
- Fix precision for dot2add. [#7861](https://github.com/microsoft/DirectXShaderCompiler/issues/7861)
- Support sign() intrinsics for unsigned integers. [#7845](https://github.com/microsoft/DirectXShaderCompiler/issues/7845)
- Fix resource heap & fvk-bind-register interactions. [#7858](https://github.com/microsoft/DirectXShaderCompiler/issues/7858)
- Fix incorrect branch gen on return. [#7834](https://github.com/microsoft/DirectXShaderCompiler/issues/7834)
- Add diagnostic for boolean bitfields. [#7722](https://github.com/microsoft/DirectXShaderCompiler/issues/7822)
- Implement WaveOpsIncludeHelperLanes. [#7806](https://github.com/microsoft/DirectXShaderCompiler/issues/7806)
- Fix spirv codegen for uabs intrinsic when argument is unsigned. [#7750](https://github.com/microsoft/DirectXShaderCompiler/issues/7750)
- Fix invalid codegen for empty cbuffers. [#7717](https://github.com/microsoft/DirectXShaderCompiler/issues/7717)
- Preserve NaN, Inf, and signed zeros w/ -Gis. [#7693](https://github.com/microsoft/DirectXShaderCompiler/issues/7693)
- Fix pcf with rich debug info enabled. [#7663](https://github.com/microsoft/DirectXShaderCompiler/issues/7663)
- Create only 1 DebugCompilationUnit per spirv module. [#7669](https://github.com/microsoft/DirectXShaderCompiler/issues/7669)
- Handle member traversal with template type. [#7674](https://github.com/microsoft/DirectXShaderCompiler/issues/7674)
- Handle partial template class specialization. [#7673](https://github.com/microsoft/DirectXShaderCompiler/issues/7673)
- Fix declaration order of values in decorations. [#7672](https://github.com/microsoft/DirectXShaderCompiler/issues/7672)
- Fix DebugSource for files which are not found. [#7662](https://github.com/microsoft/DirectXShaderCompiler/issues/7662)
- Fixed a crash if encounter constant buffer fields with overlapping register
  assignments. [#7636](https://github.com/microsoft/DirectXShaderCompiler/issues/7636)
- Add option to use the Unknown image format. [#7632](https://github.com/microsoft/DirectXShaderCompiler/issues/7632)
- Add the derivative group execution mode only on shader types that allow it. [#7628](https://github.com/microsoft/DirectXShaderCompiler/issues/7628)
- Allow spirv type as template parameter. [#7626](https://github.com/microsoft/DirectXShaderCompiler/issues/7626)
- Explicitly state which layout rules require scalar block layout. [#7539](https://github.com/microsoft/DirectXShaderCompiler/issues/7539)
- Emit DebugScope in wrapper. [#77341](https://github.com/microsoft/DirectXShaderCompiler/issues/7341)
- Use unknown image format in vk1.3 and later. [#7528](https://github.com/microsoft/DirectXShaderCompiler/issues/7528)
- Use OpCopyLogical to reconstruct values. [#7530](https://github.com/microsoft/DirectXShaderCompiler/issues/7530)
- AMD work graphs extension. [#7353](https://github.com/microsoft/DirectXShaderCompiler/issues/7353)
- Get Alignment from pointee type for vk::BufferPointer store. [#7501](https://github.com/microsoft/DirectXShaderCompiler/issues/7501)
- Fix bool cast on buffers with swizzle. [#7497](https://github.com/microsoft/DirectXShaderCompiler/issues/7497)
- Add payload to OpEmitMeshTasksEXT. [#7485](https://github.com/microsoft/DirectXShaderCompiler/issues/7485)
- Fix r-value being used in mul intrinsic. [#7489](https://github.com/microsoft/DirectXShaderCompiler/issues/7489)
- Several small bug fixes.

#### Other Changes
- Fixed regression: [#7510](https://github.com/microsoft/DirectXShaderCompiler/issues/7510) crash when calling `sizeof` on templated type.
- Fixed regression: [#7508](https://github.com/microsoft/DirectXShaderCompiler/issues/7508) crash when calling `Load` with `status`.
- Header file `dxcpix.h` was added to the release package.
- C4146 (unary minus operatore applied to unsigned type) enabled as a build
  break for MSVC builds.
- Implement isnormal function.
  [#7720](https://github.com/microsoft/DirectXShaderCompiler/issues/7720)
- Disallow 64bit msad, ibfe, and ubfe. [#7774](https://github.com/microsoft/DirectXShaderCompiler/issues/7774)
- Added support for `long long` and `unsigned long long` compile-time constant evaluation, fixes [#7952](https://github.com/microsoft/DirectXShaderCompiler/issues/7952).
- Added support for the groupshared attribute for parameters to Shader Model 6.10 [#8013](https://github.com/microsoft/DirectXShaderCompiler/pull/8013)

### Version 1.8.2505

#### Potentially breaking changes

- Typed buffers (including ROV buffers) no longer accept types other than vectors and scalars. Any other types will produce descriptive errors. This removes support for appropriately sized matrices and structs. Though it worked in some contexts, code generated from such types was unreliable.
  - Load and Store operations have been refactored as a consequence. Behavior should be identical, please file issues if discrepancies are observed.
- The compiler will now always use the internal validator instead of searching for an external DXIL.dll.  The (hidden) `-select-validator` option has been removed.

#### Notable SPIR-V updates

- Fix unnecessary Int64 requirement when loading Float64
- Added vk::BufferPointer, see [proposal](https://github.com/microsoft/hlsl-specs/blob/main/proposals/0010-vk-buffer-ref.md) for more details.
- Implement QuadAny and QuadAll (#7266)
- Fix -fvk-invert-y (#7447)

#### Shader Model 6.9 Preview

You can now compile shaders to SM 6.9, but this is a preview, so shader hashes will be set to the PREVIEW_BYPASS pattern.
SM 6.9 shaders will only work with AgilitySDK 1.717.0-preview, a supported preview driver, and use of experimental shader models in developer mode.
Preview shaders will not be compatible with the SM 6.9 release, or likely even later versions of the SM 6.9 preview.

SM 6.9 Preview Additions:

- Long vectors are allowed in HLSL when targeting shader model 6.9. Vectors up to 1024 elements in length can be loaded from/stored to raw buffers and used in elementwise operations. See the [long vector proposal](https://github.com/microsoft/hlsl-specs/blob/main/proposals/0026-hlsl-long-vector-type.md) for more details.
- HLSL Vectors are still limited to a maximum of 4 elements when used in certain contexts:
  - entry function inputs/outputs
  - parameter, payload, attribute, and node record types for mesh, raytracing, and node shaders
  - constant buffers (cbuffer), texture buffers (tbuffer), textures and typed buffers
  - Note: some HLSL elementwise intrinsics do not yet support long vectors in this preview
- Native vectors of up to 1024 elements are now present in DXIL. This includes vector llvm instructions, load/store, and various elementwise DXIL operations. This may result in smaller DXIL and potentially other performance improvements. See the [dxil vectors proposal](https://github.com/microsoft/hlsl-specs/blob/main/proposals/0030-dxil-vectors.md) for more details.
- Cooperative Vector operations, a subset of Linear Algebra (LinAlg). See the [cooperative vectors proposal](https://github.com/microsoft/hlsl-specs/blob/main/proposals/0029-cooperative-vector.md) and the [HLSL header based API proposal](https://github.com/microsoft/hlsl-specs/blob/main/proposals/0031-hlsl-vector-matrix-operations.md) for more details.
  - New built-in operations are added for multiplying long vectors with a matrix in a ByteAddressBuffer, optionally with accumulation and bias data, as well as outer product and vector accumulate operations.
  - An HLSL header shipped with this release provides a more convenient API for using these built-in operations.
- Support for [Opacity Micromaps](https://github.com/microsoft/hlsl-specs/blob/main/proposals/0024-opacity-micromaps.md) in DXR shaders as well as for RayQuery.
  - Unlocks DXR performance improvements using triangle sub-divisions for fast hit/miss detection to reduce the need for anyhit invocations.
- Support for [Shader Execution Reordering](https://github.com/microsoft/hlsl-specs/blob/main/proposals/0027-shader-execution-reordering.md) in DXR.
  - Introduces `MaybeReorderThread()` to explicitly specify where and how shader execution coherence can be improved. `MaybeReorderThread()` can be used in raygeneration shaders.
  - `HitObject` decouples traversal, intersection testing and anyhit shading from closesthit and miss shading for more control and better reordering opportunities. `HitObject` can be used in raygeneration, closesthit and miss shaders.

### Version 1.8.2502

This cumulative release contains numerous bug fixes and stability improvements.

Here are some highlights:

- The incomplete WaveMatrix implementation has been removed. [#6807](https://github.com/microsoft/DirectXShaderCompiler/pull/6807)
- DXIL Validator Hash is open sourced. [#6846](https://github.com/microsoft/DirectXShaderCompiler/pull/6846)
- DXIL container validation for PSV0 part allows any content ordering inside string and semantic index tables. [#6859](https://github.com/microsoft/DirectXShaderCompiler/pull/6859)
- The and() and or() intrinsics will now accept non-integer parameters by casting them to bools. [#7060](https://github.com/microsoft/DirectXShaderCompiler/pull/7060)
- Released executables will now expect the filenames associated with the released pdbs. Instead of expecting `dxc_full.pdb`, `dxc.exe` will now expect `dxc.pdb`.

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
