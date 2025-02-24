// RUN: %dxilver 1.8 | %dxc -T lib_6_8 %s | %D3DReflect %s | %FileCheck %s -check-prefixes=RDAT,RDAT18
// RUN: %dxilver 1.7 | %dxc -T lib_6_7 -validator-version 1.7 %s | %D3DReflect %s | FileCheck %s -check-prefixes=RDAT,RDAT17

// Ensure min shader target incorporates optional features used

// RDAT: FunctionTable[{{.*}}] = {

// SM 6.7+

///////////////////////////////////////////////////////////////////////////////
// ShaderFeatureInfo_AdvancedTextureOps (0x20000000) = 536870912

// RDAT-LABEL: UnmangledName: "sample_offset"
// RDAT:   FeatureInfo1: (AdvancedTextureOps)
// RDAT18: FeatureInfo2: (Opt_UsesDerivatives)
// Old: deriv use not tracked
// RDAT17: FeatureInfo2: 0
// RDAT18: ShaderStageFlag: (Pixel | Compute | Library | Mesh | Amplification | Node)
// Old would not report Compute, Mesh, Amplification, or Node compatibility.
// RDAT17: ShaderStageFlag: (Pixel | Library)
// RDAT18: MinShaderTarget: 0x60067
// Old: 6.0
// RDAT17: MinShaderTarget: 0x60060

Texture2D<float4> T2D : register(t0, space0);
SamplerState Samp : register(s0, space0);
RWByteAddressBuffer BAB : register(u1, space0);

[noinline] export
void sample_offset(float2 uv, int2 offsets) {
  BAB.Store(0, T2D.Sample(Samp, uv, offsets));
}

// RDAT-LABEL: UnmangledName: "sample_offset_pixel"
// RDAT18: FeatureInfo1: (AdvancedTextureOps)
// Old: missed called function
// RDAT17: FeatureInfo1: 0
// RDAT18: FeatureInfo2: (Opt_UsesDerivatives)
// Old: deriv use not tracked
// RDAT17: FeatureInfo2: 0
// RDAT: ShaderStageFlag: (Pixel)
// RDAT18: MinShaderTarget: 0x67
// Old: 6.0
// RDAT17: MinShaderTarget: 0x60

[shader("pixel")]
void sample_offset_pixel(float4 color : COLOR) {
  sample_offset(color.xy, (int)color.zw);
}
