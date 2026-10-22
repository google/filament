// RUN: %dxilver 1.8 | %dxc -T lib_6_8 %s | %D3DReflect %s | %FileCheck %s -check-prefixes=RDAT,RDAT18
// RUN: %dxilver 1.7 | %dxc -T lib_6_7 -validator-version 1.7 %s | %D3DReflect %s | FileCheck %s -check-prefixes=RDAT,RDAT17

// Ensure min shader target incorporates optional features used

// RDAT: FunctionTable[{{.*}}] = {

// SM 6.7+

///////////////////////////////////////////////////////////////////////////////
// ShaderFeatureInfo_WriteableMSAATextures (0x40000000) = 1073741824

// RDAT-LABEL: UnmangledName: "rwmsaa"
// RDAT18: FeatureInfo1: (ResourceDescriptorHeapIndexing | WriteableMSAATextures)
// Old: missed use of WriteableMSAATextures
// RDAT17: FeatureInfo1: (ResourceDescriptorHeapIndexing)
// RDAT:   FeatureInfo2: 0
// RDAT17: ShaderStageFlag: (Pixel | Vertex | Geometry | Hull | Domain | Compute | Library | RayGeneration | Intersection | AnyHit | ClosestHit | Miss | Callable | Mesh | Amplification)
// RDAT18: ShaderStageFlag: (Pixel | Vertex | Geometry | Hull | Domain | Compute | Library | RayGeneration | Intersection | AnyHit | ClosestHit | Miss | Callable | Mesh | Amplification | Node)
// RDAT18: MinShaderTarget: 0x60067
// Old: 6.6 (Because of dynamic resources)
// RDAT17: MinShaderTarget: 0x60066

RWByteAddressBuffer BAB : register(u1, space0);

[noinline] export
void rwmsaa() {
  // Use dynamic resource to avoid MSAA flag on all functions issue in 1.7
  RWTexture2DMS<float, 1> T2DMS = ResourceDescriptorHeap[0];
  uint3 whs;
  T2DMS.GetDimensions(whs.x, whs.y, whs.z);
  BAB.Store(0, whs);
}

// RDAT-LABEL: UnmangledName: "rwmsaa_in_raygen"
// RDAT18: FeatureInfo1: (ResourceDescriptorHeapIndexing | WriteableMSAATextures)
// Old: missed called function
// RDAT17: FeatureInfo1: 0
// RDAT:   FeatureInfo2: 0
// RDAT:   ShaderStageFlag: (RayGeneration)
// RDAT18: MinShaderTarget: 0x70067
// Old: 6.0
// RDAT17: MinShaderTarget: 0x70060

[shader("raygeneration")]
void rwmsaa_in_raygen() {
  rwmsaa();
}
