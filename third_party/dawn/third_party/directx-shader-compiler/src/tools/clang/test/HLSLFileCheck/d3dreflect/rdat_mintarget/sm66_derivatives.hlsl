// RUN: %dxilver 1.8 | %dxc -T lib_6_8 %s | %D3DReflect %s | %FileCheck %s -check-prefixes=RDAT,RDAT18
// RUN: %dxilver 1.7 | %dxc -T lib_6_7 -validator-version 1.7 %s | %D3DReflect %s | FileCheck %s -check-prefixes=RDAT,RDAT17
// RUN: %dxilver 1.6 | %dxc -T lib_6_6 -validator-version 1.6 %s | %D3DReflect %s | FileCheck %s -check-prefixes=RDAT,RDAT16

// Compile to DXIL to check ShaderFlags.
// RUN: %dxilver 1.8 | %dxc -T lib_6_8 %s | %FileCheck %s -check-prefixes=DXIL,DXIL18
// RUN: %dxilver 1.7 | %dxc -T lib_6_7 -validator-version 1.7 %s | FileCheck %s -check-prefixes=DXIL,DXIL17
// RUN: %dxilver 1.6 | %dxc -T lib_6_6 -validator-version 1.6 %s | FileCheck %s -check-prefixes=DXIL,DXIL16

///////////////////////////////////////////////////////////////////////////////
// DXIL checks
// Ensure ShaderFlags do not include UsedsDerivatives, but do include
// DerivativesInMeshAndAmpShaders in validator version 1.8 only.  1.8 should
// no longer include UAVsAtEveryStage, since that shouldn't have applied to
// library profile, only VS, HS, DS, and GS profiles.
// These flag values are raw shader flags, so they are not the same as the
// feature flag values.

// DXIL: !dx.entryPoints = !{![[lib_entry:[0-9]+]],
// DXIL: ![[lib_entry]] = !{null, !"", null, !{{.*}}, ![[shader_flags:[0-9]+]]}
// 65552 = 0x10010 = EnableRawAndStructuredBuffers(0x10) + UAVsAtEveryStage(0x10000)
// DXIL16: ![[shader_flags]] = !{i32 0, i64 65552}
// 8590000144 = 0x200010010 = 0x10010 + ResMayNotAlias(0x200000000)
// DXIL17: ![[shader_flags]] = !{i32 0, i64 8590000144}
// 9126805520 = 0x220000010 = 0x200010010 - UAVsAtEveryStage(0x10000) +
//    DerivativesInMeshAndAmpShaders(0x20000000)
// DXIL18: ![[shader_flags]] = !{i32 0, i64 9126805520}

///////////////////////////////////////////////////////////////////////////////
// RDAT checks
// Ensure min shader target incorporates optional features used

// RDAT: FunctionTable[{{.*}}] = {

// SM 6.6+

///////////////////////////////////////////////////////////////////////////////
// ShaderFeatureInfo_DerivativesInMeshAndAmpShaders (0x1000000) = 16777216
// OptFeatureInfo_UsesDerivatives (0x0000010000000000) = FeatureInfo2: 256

// OptFeatureInfo_UsesDerivatives Flag used to indicate derivative use in
// functions, then fixed up for entry functions.
// Val. ver. 1.8 required to recursively check called functions.

// RDAT-LABEL: UnmangledName: "deriv_in_func"
// RDAT:   FeatureInfo1: 0
// RDAT18: FeatureInfo2: (Opt_UsesDerivatives)
// Old: deriv use not tracked
// RDAT16: FeatureInfo2: 0
// RDAT17: FeatureInfo2: 0
// RDAT18: ShaderStageFlag: (Pixel | Compute | Library | Mesh | Amplification | Node)
// Old would not report Compute, Mesh, Amplification, or Node compatibility.
// RDAT16: ShaderStageFlag: (Pixel | Library)
// RDAT17: ShaderStageFlag: (Pixel | Library)
// RDAT17: MinShaderTarget: 0x60060
// RDAT18: MinShaderTarget: 0x60060
// Old: Didn't set min target properly for lib function
// RDAT16: MinShaderTarget: 0x60066

RWByteAddressBuffer BAB : register(u1, space0);

[noinline] export
void deriv_in_func(float2 uv) {
  BAB.Store(0, ddx(uv));
}

// RDAT-LABEL: UnmangledName: "deriv_in_mesh"
// RDAT18: FeatureInfo1: (DerivativesInMeshAndAmpShaders)
// Old: missed called function
// RDAT16: FeatureInfo1: 0
// RDAT17: FeatureInfo1: 0
// RDAT18: FeatureInfo2: (Opt_UsesDerivatives)
// Old: deriv use not tracked
// RDAT16: FeatureInfo2: 0
// RDAT17: FeatureInfo2: 0
// Mesh(13) = 0x2000 = 8192
// RDAT:   ShaderStageFlag: (Mesh)
// RDAT18: MinShaderTarget: 0xd0066
// Old: 6.0
// RDAT16: MinShaderTarget: 0xd0060
// RDAT17: MinShaderTarget: 0xd0060

[shader("mesh")]
[numthreads(8, 8, 1)]
[outputtopology("triangle")]
void deriv_in_mesh(uint3 DTid : SV_DispatchThreadID) {
  float2 uv = DTid.xy/float2(8, 8);
  deriv_in_func(uv);
}

// RDAT-LABEL: UnmangledName: "deriv_in_compute"
// RDAT:   FeatureInfo1: 0
// RDAT18: FeatureInfo2: (Opt_UsesDerivatives)
// Old: deriv use not tracked
// RDAT16: FeatureInfo2: 0
// RDAT17: FeatureInfo2: 0
// RDAT:   ShaderStageFlag: (Compute)
// RDAT18: MinShaderTarget: 0x50066
// Old: 6.0
// RDAT16: MinShaderTarget: 0x50060
// RDAT17: MinShaderTarget: 0x50060

[shader("compute")]
[numthreads(8, 8, 1)]
void deriv_in_compute(uint3 DTid : SV_DispatchThreadID) {
  float2 uv = DTid.xy/float2(8, 8);
  deriv_in_func(uv);
}

// RDAT-LABEL: UnmangledName: "deriv_in_pixel"
// RDAT:   FeatureInfo1: 0
// RDAT18: FeatureInfo2: (Opt_UsesDerivatives)
// Old: deriv use not tracked
// RDAT16: FeatureInfo2: 0
// RDAT17: FeatureInfo2: 0
// Pixel(0) = 0x1 = 1
// RDAT:   ShaderStageFlag: (Pixel)
// RDAT:   MinShaderTarget: 0x60

[shader("pixel")]
void deriv_in_pixel(float2 uv : TEXCOORD) {
  deriv_in_func(uv);
}

// Make sure function-level derivative flag isn't in RequiredFeatureFlags,
// and make sure mesh shader sets required flag.

// RDAT-LABEL: ID3D12LibraryReflection:

// RDAT-LABEL: D3D12_FUNCTION_DESC: Name:
// RDAT-SAME: deriv_in_func
// RDAT:   RequiredFeatureFlags: 0

// RDAT-LABEL: D3D12_FUNCTION_DESC: Name: deriv_in_compute
// RDAT:   RequiredFeatureFlags: 0

// RDAT-LABEL: D3D12_FUNCTION_DESC: Name: deriv_in_mesh
// ShaderFeatureInfo_DerivativesInMeshAndAmpShaders (0x1000000) = 16777216
// RDAT18: RequiredFeatureFlags: 0x1000000
// Old: missed called function
// RDAT16: RequiredFeatureFlags: 0
// RDAT17: RequiredFeatureFlags: 0

// RDAT-LABEL: D3D12_FUNCTION_DESC: Name: deriv_in_pixel
// RDAT:   RequiredFeatureFlags: 0
