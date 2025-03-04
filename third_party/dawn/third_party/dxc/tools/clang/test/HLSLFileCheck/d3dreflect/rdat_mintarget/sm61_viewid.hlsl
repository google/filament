// RUN: %dxilver 1.8 | %dxc -T lib_6_8 %s | %D3DReflect %s | %FileCheck %s -check-prefixes=RDAT
// RUN: %dxilver 1.7 | %dxc -T lib_6_7 -validator-version 1.7 %s | %D3DReflect %s | FileCheck %s -check-prefixes=RDAT

// Ensure min shader target incorporates optional features used

// RDAT: FunctionTable[{{.*}}] = {

// SM 6.1+

///////////////////////////////////////////////////////////////////////////////
// ShaderFeatureInfo_ViewID (0x10000) = 65536

// ViewID is loaded using an intrinsic, so prior validator already adjusted SM
// for it.

// RDAT-LABEL: UnmangledName: "viewid"
// RDAT:   FeatureInfo1: (ViewID)
// RDAT:   FeatureInfo2: 0
// RDAT:   ShaderStageFlag: (Pixel)
// RDAT:   MinShaderTarget: 0x61

[shader("pixel")]
void viewid(uint vid : SV_ViewID, out float4 target : SV_Target) {
  target = vid;
}
