// RUN: %dxilver 1.8 | %dxc -T lib_6_8 %s | %D3DReflect %s | %FileCheck %s -check-prefixes=RDAT

// Ensure min shader target incorporates shader stage of entry function
// These must be minimal shaders since intrinsic usage associated with the
// shader stage will cause the min target to be set that way.

// This covers node shaders, which should always be SM 6.8+

// RDAT: FunctionTable[{{.*}}] = {

RWByteAddressBuffer BAB : register(u1, space0);

// RDAT-LABEL: UnmangledName: "node1"
// RDAT:   FeatureInfo1: 0
// RDAT:   FeatureInfo2: 0
// RDAT: ShaderStageFlag: (Node)
// RDAT: MinShaderTarget: 0xf0068

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(1,1,1)]
void node1() {
  BAB.Store(0, 0);
}
