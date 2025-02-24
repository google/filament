// RUN: %dxilver 1.8 | %dxc -T lib_6_8 %s | %D3DReflect %s | %FileCheck %s -check-prefixes=RDAT

// Ensure min shader target incorporates optional features used

// RDAT: FunctionTable[{{.*}}] = {

// SM 6.8+

///////////////////////////////////////////////////////////////////////////////
// ShaderFeatureInfo_SampleCmpGradientOrBias (0x80000000) = 2147483648

Texture2D<float4> T2D : register(t0, space0);
SamplerComparisonState SampCmp : register(s0, space0);
RWByteAddressBuffer BAB : register(u1, space0);

// RDAT-LABEL: UnmangledName: "samplecmpgrad"
// RDAT:   FeatureInfo1: (SampleCmpGradientOrBias)
// RDAT:   FeatureInfo2: 0
// RDAT:   ShaderStageFlag: (Pixel | Vertex | Geometry | Hull | Domain | Compute | Library | RayGeneration | Intersection | AnyHit | ClosestHit | Miss | Callable | Mesh | Amplification | Node)
// RDAT:   MinShaderTarget: 0x60068

[noinline] export
void samplecmpgrad() {
  // Use SampleCmpGrad in a minimal way that survives dead code elimination.
  float4 tex = T2D.SampleCmpGrad(
      SampCmp, float2(0.5, 0.5), 0.5,
      float2(0.005, 0.005), float2(0.005, 0.005));
  BAB.Store(0, tex.x);
}

// RDAT-LABEL: UnmangledName: "samplecmpgrad_compute"
// RDAT:   FeatureInfo1: (SampleCmpGradientOrBias)
// RDAT:   FeatureInfo2: 0
// RDAT:   ShaderStageFlag: (Compute)
// RDAT:   MinShaderTarget: 0x50068

[shader("compute")]
[numthreads(1,1,1)]
void samplecmpgrad_compute(uint tidx : SV_GroupIndex) {
  samplecmpgrad();
}

// RDAT-LABEL: UnmangledName: "samplecmpbias"
// RDAT:   FeatureInfo1: (SampleCmpGradientOrBias)
// RDAT:   FeatureInfo2: (Opt_UsesDerivatives)
// RDAT:   ShaderStageFlag: (Pixel | Compute | Library | Mesh | Amplification | Node)
// RDAT:   MinShaderTarget: 0x60068

[noinline] export
void samplecmpbias() {
  // Use SampleCmpGrad in a minimal way that survives dead code elimination.
  float4 tex = T2D.SampleCmpBias(SampCmp, float2(0.5, 0.5), 0.5, 0.5);
  BAB.Store(0, tex.x);
}

// RDAT-LABEL: UnmangledName: "samplecmpbias_compute"
// RDAT:   FeatureInfo1: (SampleCmpGradientOrBias)
// RDAT:   FeatureInfo2: (Opt_UsesDerivatives)
// RDAT:   ShaderStageFlag: (Compute)
// RDAT:   MinShaderTarget: 0x50068

[shader("compute")]
[numthreads(4,1,1)]
void samplecmpbias_compute(uint tidx : SV_GroupIndex) {
  samplecmpbias();
}
