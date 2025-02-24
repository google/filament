// RUN: %dxilver 1.8 | %dxc -T lib_6_8 %s | %D3DReflect %s | %FileCheck %s -check-prefixes=RDAT

// Ensure min shader target incorporates optional features used

// RDAT: FunctionTable[{{.*}}] = {

// SM 6.8+

///////////////////////////////////////////////////////////////////////////////
// ShaderFeatureInfo_ExtendedCommandInfo (0x100000000) = FeatureInfo2: 1

RWByteAddressBuffer BAB : register(u1, space0);

// These are loaded using an intrinsic, SM adjusted by dxil op as well.

// RDAT-LABEL: UnmangledName: "startvertexlocation"
// RDAT: FeatureInfo1: 0
// RDAT: FeatureInfo2: (ExtendedCommandInfo)
// RDAT: ShaderStageFlag: (Vertex)
// RDAT: MinShaderTarget: 0x10068

[shader("vertex")]
void startvertexlocation(uint u : SV_StartVertexLocation) {
  BAB.Store(0, u);
}

// RDAT-LABEL: UnmangledName: "startinstancelocation"
// RDAT: FeatureInfo1: 0
// RDAT: FeatureInfo2: (ExtendedCommandInfo)
// RDAT: ShaderStageFlag: (Vertex)
// RDAT: MinShaderTarget: 0x10068

[shader("vertex")]
void startinstancelocation(uint u : SV_StartInstanceLocation) {
  BAB.Store(0, u);
}
