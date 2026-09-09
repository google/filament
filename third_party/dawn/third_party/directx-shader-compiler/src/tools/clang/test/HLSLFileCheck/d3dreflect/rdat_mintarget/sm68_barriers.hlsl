// RUN: %dxilver 1.8 | %dxc -T lib_6_8 %s | %D3DReflect %s | %FileCheck %s -check-prefixes=RDAT

// Check that stage flags are set correctly for different barrier modes with
// new the SM 6.8 barrier intrinsics.

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

// RDAT-LABEL: UnmangledName: "fn_barrier_device1"
// RDAT: FeatureInfo1: 0
// RDAT: FeatureInfo2: 0
// RDAT: ShaderStageFlag: (Pixel | Vertex | Geometry | Hull | Domain | Compute | Library | RayGeneration | Intersection | AnyHit | ClosestHit | Miss | Callable | Mesh | Amplification | Node)
// RDAT: MinShaderTarget: 0x60060

[noinline] export
void fn_barrier_device1() {
  Barrier(UAV_MEMORY, DEVICE_SCOPE);
}

// RDAT-LABEL: UnmangledName: "fn_barrier_device2"
// RDAT: FeatureInfo1: 0
// RDAT: FeatureInfo2: 0
// RDAT: ShaderStageFlag: (Pixel | Vertex | Geometry | Hull | Domain | Compute | Library | RayGeneration | Intersection | AnyHit | ClosestHit | Miss | Callable | Mesh | Amplification | Node)
// RDAT: MinShaderTarget: 0x60068

[noinline] export
void fn_barrier_device2() {
  Barrier(BAB, DEVICE_SCOPE);
}

// RDAT-LABEL: UnmangledName: "fn_barrier_group1"
// RDAT: FeatureInfo1: 0
// RDAT: FeatureInfo2: (Opt_RequiresGroup)
// RDAT: ShaderStageFlag: (Compute | Library | Mesh | Amplification | Node)
// RDAT: MinShaderTarget: 0x60060

[noinline] export
void fn_barrier_group1() {
  Barrier(GROUP_SHARED_MEMORY, GROUP_SYNC | GROUP_SCOPE);
}

// RDAT-LABEL: UnmangledName: "fn_barrier_group2"
// RDAT: FeatureInfo1: 0
// RDAT: FeatureInfo2: (Opt_RequiresGroup)
// RDAT: ShaderStageFlag: (Compute | Library | Mesh | Amplification | Node)
// RDAT: MinShaderTarget: 0x60068

[noinline] export
void fn_barrier_group2() {
  Barrier(BAB, GROUP_SYNC | GROUP_SCOPE);
}

// RDAT-LABEL: UnmangledName: "fn_barrier_node1"
// RDAT: FeatureInfo1: 0
// RDAT: FeatureInfo2: 0
// RDAT: ShaderStageFlag: (Library | Node)
// RDAT: MinShaderTarget: 0x60068

[noinline] export
void fn_barrier_node1() {
  Barrier(NODE_INPUT_MEMORY, DEVICE_SCOPE);
}

// RDAT-LABEL: UnmangledName: "fn_barrier_node_group1"
// RDAT: FeatureInfo1: 0
// RDAT: FeatureInfo2: (Opt_RequiresGroup)
// RDAT: ShaderStageFlag: (Library | Node)
// RDAT: MinShaderTarget: 0x60068

[noinline] export
void fn_barrier_node_group1() {
  Barrier(NODE_INPUT_MEMORY, GROUP_SYNC | GROUP_SCOPE);
}

// RDAT-LABEL: UnmangledName: "node_barrier_device_in_call"
// RDAT: FeatureInfo1: 0
// RDAT: FeatureInfo2: 0
// RDAT: ShaderStageFlag: (Node)
// RDAT: MinShaderTarget: 0xf0068

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(1,1,1)]
void node_barrier_device_in_call() {
  fn_barrier_device1();
  BAB.Store(0, 0);
}

// RDAT-LABEL: UnmangledName: "node_barrier_node_in_call"
// RDAT: FeatureInfo1: 0
// RDAT: FeatureInfo2: 0
// RDAT: ShaderStageFlag: (Node)
// RDAT: MinShaderTarget: 0xf0068

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(1,1,1)]
void node_barrier_node_in_call() {
  fn_barrier_node1();
  BAB.Store(0, 0);
}

// RDAT-LABEL: UnmangledName: "node_barrier_node_group_in_call"
// RDAT: FeatureInfo1: 0
// RDAT: FeatureInfo2: (Opt_RequiresGroup)
// RDAT: ShaderStageFlag: (Node)
// RDAT: MinShaderTarget: 0xf0068

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(1,1,1)]
void node_barrier_node_group_in_call() {
  fn_barrier_node_group1();
  BAB.Store(0, 0);
}
