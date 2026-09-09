// RUN: %dxilver 1.8 | %dxc -T lib_6_8 %s | %D3DReflect %s | %FileCheck %s -check-prefixes=RDAT

// Ensure that barrier is allowed for node shaders, and that correct shader
// flags are set.

// RDAT: FunctionTable[{{.*}}] = {

RWByteAddressBuffer BAB : register(u1, space0);

// RDAT-LABEL: UnmangledName: "node_barrier"
// RDAT: FeatureInfo1: 0
// RDAT: FeatureInfo2: (Opt_RequiresGroup)
// RDAT: ShaderStageFlag: (Node)
// RDAT: MinShaderTarget: 0xf0068

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(1,1,1)]
void node_barrier() {
  GroupMemoryBarrierWithGroupSync();
  BAB.Store(0, 0);
}

// RDAT-LABEL: UnmangledName: "use_barrier"
// RDAT: FeatureInfo1: 0
// RDAT: FeatureInfo2: (Opt_RequiresGroup)
// RDAT: ShaderStageFlag: (Compute | Library | Mesh | Amplification | Node)
// RDAT: MinShaderTarget: 0x60060

[noinline] export
void use_barrier() {
  GroupMemoryBarrierWithGroupSync();
}

// RDAT-LABEL: UnmangledName: "node_barrier_in_call"
// RDAT: FeatureInfo1: 0
// RDAT: FeatureInfo2: (Opt_RequiresGroup)
// RDAT: ShaderStageFlag: (Node)
// RDAT: MinShaderTarget: 0xf0068

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(1,1,1)]
void node_barrier_in_call() {
  use_barrier();
  BAB.Store(0, 0);
}
