// RUN: %dxilver 1.8 | %dxc -T lib_6_8 %s | %D3DReflect %s | %FileCheck %s -check-prefixes=RDAT,RDAT18
// RUN: %dxilver 1.7 | %dxc -T lib_6_7 -validator-version 1.7 %s | %D3DReflect %s | FileCheck %s -check-prefixes=RDAT,RDAT17


// Ensure min shader target incorporates optional features used

// RDAT: FunctionTable[{{.*}}] = {

// SM 6.7+

///////////////////////////////////////////////////////////////////////////////
// ShaderFeatureInfo_WriteableMSAATextures (0x40000000) = 1073741824

// RDAT-LABEL: UnmangledName: "rwmsaa"
// RDAT:   FeatureInfo1: (WriteableMSAATextures)
// RDAT:   FeatureInfo2: 0
// RDAT17: ShaderStageFlag: (Pixel | Vertex | Geometry | Hull | Domain | Compute | Library | RayGeneration | Intersection | AnyHit | ClosestHit | Miss | Callable | Mesh | Amplification)
// RDAT18: ShaderStageFlag: (Pixel | Vertex | Geometry | Hull | Domain | Compute | Library | RayGeneration | Intersection | AnyHit | ClosestHit | Miss | Callable | Mesh | Amplification | Node)
// RDAT18: MinShaderTarget: 0x60067
// Old: 6.0
// RDAT17: MinShaderTarget: 0x60060

RWByteAddressBuffer BAB : register(u1, space0);
RWTexture2DMS<float, 1> T2DMS : register(u2, space0);

[noinline] export
void rwmsaa() {
  BAB.Store(0, T2DMS.sample[1][uint2(1, 2)]);
}

// RDAT-LABEL: UnmangledName: "no_rwmsaa"
// RDAT18: FeatureInfo1: 0
// 1.7 Incorrectly reports feature use for function
// RDAT17: FeatureInfo1: (WriteableMSAATextures)
// RDAT:   FeatureInfo2: 0
// RDAT17: ShaderStageFlag: (Pixel | Vertex | Geometry | Hull | Domain | Compute | Library | RayGeneration | Intersection | AnyHit | ClosestHit | Miss | Callable | Mesh | Amplification)
// RDAT18: ShaderStageFlag: (Pixel | Vertex | Geometry | Hull | Domain | Compute | Library | RayGeneration | Intersection | AnyHit | ClosestHit | Miss | Callable | Mesh | Amplification | Node)
// RDAT: MinShaderTarget: 0x60060

export void no_rwmsaa() {
  BAB.Store(0, 0);
}

// RDAT-LABEL: UnmangledName: "rwmsaa_in_raygen"
// Old: set because of global
// RDAT:   FeatureInfo1: (WriteableMSAATextures)
// RDAT:   FeatureInfo2: 0
// RDAT:   ShaderStageFlag: (RayGeneration)
// RDAT18: MinShaderTarget: 0x70067
// Old: 6.0
// RDAT17: MinShaderTarget: 0x70060

[shader("raygeneration")]
void rwmsaa_in_raygen() {
  rwmsaa();
}

// RDAT-LABEL: UnmangledName: "rwmsaa_not_used_in_raygen"
// RDAT18: FeatureInfo1: 0
// Old: set because of global
// RDAT17: FeatureInfo1: (WriteableMSAATextures)
// RDAT:   FeatureInfo2: 0
// RDAT:   ShaderStageFlag: (RayGeneration)
// RDAT18: MinShaderTarget: 0x70063
// Old: 6.0
// RDAT17: MinShaderTarget: 0x70060

[shader("raygeneration")]
void rwmsaa_not_used_in_raygen() {
  no_rwmsaa();
}
