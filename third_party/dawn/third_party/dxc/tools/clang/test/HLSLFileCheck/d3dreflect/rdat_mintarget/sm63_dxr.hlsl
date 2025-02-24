// RUN: %dxilver 1.8 | %dxc -T lib_6_8 %s | %D3DReflect %s | %FileCheck %s -check-prefixes=RDAT,RDAT18
// RUN: %dxilver 1.7 | %dxc -T lib_6_7 -validator-version 1.7 %s | %D3DReflect %s | FileCheck %s -check-prefixes=RDAT,RDAT17

// Ensure min shader target incorporates shader stage of entry function
// These must be minimal shaders since intrinsic usage associated with the
// shader stage will cause the min target to be set that way.

// This covers raytracing entry points, which should always be SM 6.3+

// RDAT: FunctionTable[{{.*}}] = {

RWByteAddressBuffer BAB : register(u1, space0);

// RDAT-LABEL: UnmangledName: "raygen"
// RDAT:   FeatureInfo1: 0
// RDAT:   FeatureInfo2: 0
// RDAT:   ShaderStageFlag: (RayGeneration)
// RDAT18: MinShaderTarget: 0x70063
// Old: 6.0
// RDAT17: MinShaderTarget: 0x70060

[shader("raygeneration")]
void raygen() {
  BAB.Store(0, 0);
}

// RDAT-LABEL: UnmangledName: "intersection"
// RDAT:   FeatureInfo1: 0
// RDAT:   FeatureInfo2: 0
// RDAT:   ShaderStageFlag: (Intersection)
// RDAT18: MinShaderTarget: 0x80063
// Old: 6.0
// RDAT17: MinShaderTarget: 0x80060

[shader("intersection")]
void intersection() {
  BAB.Store(0, 0);
}

// RDAT-LABEL: UnmangledName: "anyhit"
// RDAT:   FeatureInfo1: 0
// RDAT:   FeatureInfo2: 0
// RDAT:   ShaderStageFlag: (AnyHit)
// RDAT18: MinShaderTarget: 0x90063
// Old: 6.0
// RDAT17: MinShaderTarget: 0x90060

struct [raypayload] MyPayload {
  float2 loc : write(caller) : read(caller);
};

[shader("anyhit")]
void anyhit(inout MyPayload payload : SV_RayPayload,
    in BuiltInTriangleIntersectionAttributes attr : SV_IntersectionAttributes ) {
  BAB.Store(0, 0);
}

// RDAT-LABEL: UnmangledName: "closesthit"
// RDAT:   FeatureInfo1: 0
// RDAT:   FeatureInfo2: 0
// RDAT:   ShaderStageFlag: (ClosestHit)
// RDAT18: MinShaderTarget: 0xa0063
// Old: 6.0
// RDAT17: MinShaderTarget: 0xa0060

[shader("closesthit")]
void closesthit(inout MyPayload payload : SV_RayPayload,
    in BuiltInTriangleIntersectionAttributes attr : SV_IntersectionAttributes ) {
  BAB.Store(0, 0);
}

// RDAT-LABEL: UnmangledName: "miss"
// RDAT:   FeatureInfo1: 0
// RDAT:   FeatureInfo2: 0
// RDAT:   ShaderStageFlag: (Miss)
// RDAT18: MinShaderTarget: 0xb0063
// Old: 6.0
// RDAT17: MinShaderTarget: 0xb0060

[shader("miss")]
void miss(inout MyPayload payload : SV_RayPayload) {
  BAB.Store(0, 0);
}

// RDAT-LABEL: UnmangledName: "callable"
// RDAT:   FeatureInfo1: 0
// RDAT:   FeatureInfo2: 0
// RDAT:   ShaderStageFlag: (Callable)
// RDAT18: MinShaderTarget: 0xc0063
// Old: 6.0
// RDAT17: MinShaderTarget: 0xc0060

[shader("callable")]
void callable(inout MyPayload param) {
  BAB.Store(0, 0);
}
