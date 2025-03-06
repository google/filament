// RUN: %dxilver 1.8 | %dxc -T lib_6_8 %s | %D3DReflect %s | %FileCheck %s -check-prefixes=RDAT
// RUN: %dxilver 1.7 | %dxc -T lib_6_7 -validator-version 1.7 %s | %D3DReflect %s | FileCheck %s -check-prefixes=RDAT

// Ensure min shader target incorporates optional features used

// RDAT: FunctionTable[{{.*}}] = {

// SM 6.1+

///////////////////////////////////////////////////////////////////////////////
// ShaderFeatureInfo_Barycentrics (0x20000) = 131072

// RDAT-LABEL: UnmangledName: "bary1"
// RDAT:   FeatureInfo1: (Barycentrics)
// RDAT:   FeatureInfo2: 0
// RDAT:   ShaderStageFlag: (Pixel)
// RDAT:   MinShaderTarget: 0x61

[shader("pixel")]
void bary1(float3 barycentrics : SV_Barycentrics, out float4 target : SV_Target) {
  target = float4(barycentrics, 1);
}

// RDAT-LABEL: UnmangledName: "bary2"
// RDAT:   FeatureInfo1: (Barycentrics)
// RDAT:   FeatureInfo2: 0
// RDAT:   ShaderStageFlag: (Pixel)
// RDAT:   MinShaderTarget: 0x61

[shader("pixel")]
void bary2(nointerpolation float4 color : COLOR, out float4 target : SV_Target) {
  target = GetAttributeAtVertex(color, 1);
}
