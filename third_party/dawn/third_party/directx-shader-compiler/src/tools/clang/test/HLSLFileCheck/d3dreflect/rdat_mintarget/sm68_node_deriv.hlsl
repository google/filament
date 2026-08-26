// RUN: %dxilver 1.8 | %dxc -T lib_6_8 %s | %D3DReflect %s | %FileCheck %s -check-prefixes=RDAT

// Ensure that categories of deriv ops are allowed for node shaders.
// Ensure that the OptFeatureInfo_UsesDerivatives flag is set as well.

// RDAT: FunctionTable[{{.*}}] = {

Texture2D<float4> T2D : register(t0, space0);
SamplerState Samp : register(s0, space0);
RWByteAddressBuffer BAB : register(u1, space0);

///////////////////////////////////////////////////////////////////////////////
// Category: derivatives ddx/ddy/ddx_coarse/ddy_coarse/ddx_fine/ddy_fine

// RDAT-LABEL: UnmangledName: "node_deriv"
// RDAT: FeatureInfo1: 0
// RDAT: FeatureInfo2: (Opt_UsesDerivatives)
// RDAT: ShaderStageFlag: (Node)
// RDAT: MinShaderTarget: 0xf0068

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(4,4,1)]
void node_deriv(uint3 tid : SV_GroupThreadID) {
  float2 uv = tid.xy / float2(4, 4);
  float2 ddx_uv = ddx(uv);
  BAB.Store(0, ddx_uv);
}

// RDAT-LABEL: UnmangledName: "use_deriv"
// RDAT: FeatureInfo1: 0
// RDAT: FeatureInfo2: (Opt_UsesDerivatives)
// RDAT: ShaderStageFlag: (Pixel | Compute | Library | Mesh | Amplification | Node)
// RDAT: MinShaderTarget: 0x60060

[noinline] export
void use_deriv(float2 uv) {
  float2 ddx_uv = ddx(uv);
  BAB.Store(0, ddx_uv);
}

// RDAT-LABEL: UnmangledName: "node_deriv_in_call"
// RDAT: FeatureInfo1: 0
// RDAT: FeatureInfo2: (Opt_UsesDerivatives)
// RDAT: ShaderStageFlag: (Node)
// RDAT: MinShaderTarget: 0xf0068

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(4,4,1)]
void node_deriv_in_call(uint3 tid : SV_GroupThreadID) {
  float2 uv = tid.xy / float2(4, 4);
  use_deriv(uv);
}

///////////////////////////////////////////////////////////////////////////////
// Category: CalculateLOD

// RDAT-LABEL: UnmangledName: "node_calclod"
// RDAT: FeatureInfo1: 0
// RDAT: FeatureInfo2: (Opt_UsesDerivatives)
// RDAT: ShaderStageFlag: (Node)
// RDAT: MinShaderTarget: 0xf0068

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(4,4,1)]
void node_calclod(uint3 tid : SV_GroupThreadID) {
  float2 uv = tid.xy / float2(4, 4);
  float lod = T2D.CalculateLevelOfDetail(Samp, uv);
  BAB.Store(0, lod);
}

// RDAT-LABEL: UnmangledName: "use_calclod"
// RDAT: FeatureInfo1: 0
// RDAT: FeatureInfo2: (Opt_UsesDerivatives)
// RDAT: ShaderStageFlag: (Pixel | Compute | Library | Mesh | Amplification | Node)
// RDAT: MinShaderTarget: 0x60060

[noinline] export
void use_calclod(float2 uv) {
  float lod = T2D.CalculateLevelOfDetail(Samp, uv);
  BAB.Store(0, lod);
}

// RDAT-LABEL: UnmangledName: "node_calclod_in_call"
// RDAT: FeatureInfo1: 0
// RDAT: FeatureInfo2: (Opt_UsesDerivatives)
// RDAT: ShaderStageFlag: (Node)
// RDAT: MinShaderTarget: 0xf0068

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(4,4,1)]
void node_calclod_in_call(uint3 tid : SV_GroupThreadID) {
  float2 uv = tid.xy / float2(4, 4);
  use_calclod(uv);
}

///////////////////////////////////////////////////////////////////////////////
// Category: Sample with implicit derivatives

// RDAT-LABEL: UnmangledName: "node_sample"
// RDAT: FeatureInfo1: 0
// RDAT: FeatureInfo2: (Opt_UsesDerivatives)
// RDAT: ShaderStageFlag: (Node)
// RDAT: MinShaderTarget: 0xf0068

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(4,4,1)]
void node_sample(uint3 tid : SV_GroupThreadID) {
  float2 uv = tid.xy / float2(4, 4);
  float4 color = T2D.Sample(Samp, uv);
  BAB.Store(0, color);
}

// RDAT-LABEL: UnmangledName: "use_sample"
// RDAT: FeatureInfo1: 0
// RDAT: FeatureInfo2: (Opt_UsesDerivatives)
// RDAT: ShaderStageFlag: (Pixel | Compute | Library | Mesh | Amplification | Node)
// RDAT: MinShaderTarget: 0x60060

[noinline] export
void use_sample(float2 uv) {
  float4 color = T2D.Sample(Samp, uv);
  BAB.Store(0, color);
}

// RDAT-LABEL: UnmangledName: "node_sample_in_call"
// RDAT: FeatureInfo1: 0
// RDAT: FeatureInfo2: (Opt_UsesDerivatives)
// RDAT: ShaderStageFlag: (Node)
// RDAT: MinShaderTarget: 0xf0068

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(4,4,1)]
void node_sample_in_call(uint3 tid : SV_GroupThreadID) {
  float2 uv = tid.xy / float2(4, 4);
  use_sample(uv);
}

///////////////////////////////////////////////////////////////////////////////
// Category: Quad ops
// Quad ops do not set the UsesDerivatives flag, only requiring wave ops flag.

// RDAT-LABEL: UnmangledName: "node_quad"
// RDAT: FeatureInfo1: (WaveOps)
// RDAT: FeatureInfo2: 0
// RDAT: MinShaderTarget: 0xf0068

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(4,4,1)]
void node_quad(uint3 tid : SV_GroupThreadID) {
  float2 uv = tid.xy / float2(4, 4);
  float2 result = QuadReadAcrossDiagonal(uv);
  BAB.Store(0, result);
}

// RDAT-LABEL: UnmangledName: "use_quad"
// RDAT: FeatureInfo1: (WaveOps)
// RDAT: FeatureInfo2: 0
// RDAT: ShaderStageFlag: (Pixel | Compute | Library | Mesh | Amplification | Node)
// RDAT: MinShaderTarget: 0x60060

[noinline] export
void use_quad(float2 uv) {
  float2 result = QuadReadAcrossDiagonal(uv);
  BAB.Store(0, result);
}

// RDAT-LABEL: UnmangledName: "node_quad_in_call"
// RDAT: FeatureInfo1: (WaveOps)
// RDAT: FeatureInfo2: 0
// RDAT: ShaderStageFlag: (Node)
// RDAT: MinShaderTarget: 0xf0068

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(4,4,1)]
void node_quad_in_call(uint3 tid : SV_GroupThreadID) {
  float2 uv = tid.xy / float2(4, 4);
  use_quad(uv);
}
