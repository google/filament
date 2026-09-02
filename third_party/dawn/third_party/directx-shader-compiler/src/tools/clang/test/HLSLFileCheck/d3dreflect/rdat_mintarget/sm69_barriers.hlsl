// RUN: %dxilver 1.9 | %dxc -T lib_6_9 %s | %D3DReflect %s | %FileCheck %s -check-prefixes=RDAT

// Check that stage flags are set correctly still for different barrier modes in SM 6.9.

// RDAT: FunctionTable[{{.*}}] = {

RWByteAddressBuffer BAB : register(u1, space0);

// RDAT-LABEL: UnmangledName: "fn_barrier_reorder"
// RDAT: FeatureInfo1: 0
// RDAT: FeatureInfo2: 0
// RDAT: ShaderStageFlag: (Library | RayGeneration)
// RDAT: MinShaderTarget: 0x60069

[noinline] export
void fn_barrier_reorder() {
  Barrier(UAV_MEMORY, REORDER_SCOPE);
}

// RDAT-LABEL: UnmangledName: "fn_barrier_reorder2"
// RDAT: FeatureInfo1: 0
// RDAT: FeatureInfo2: 0
// RDAT: ShaderStageFlag: (Library | RayGeneration)
// RDAT: MinShaderTarget: 0x60069

[noinline] export
void fn_barrier_reorder2() {
  Barrier(BAB, REORDER_SCOPE);
}

// RDAT-LABEL: UnmangledName: "rg_barrier_reorder_in_call"
// RDAT: FeatureInfo1: 0
// RDAT: FeatureInfo2: 0
// RDAT: ShaderStageFlag: (RayGeneration)
// RDAT: MinShaderTarget: 0x70069

[shader("raygeneration")]
void rg_barrier_reorder_in_call() {
  fn_barrier_reorder();
  BAB.Store(0, 0);
}

// RDAT-LABEL: UnmangledName: "rg_barrier_reorder_in_call2"
// RDAT: FeatureInfo1: 0
// RDAT: FeatureInfo2: 0
// RDAT: ShaderStageFlag: (RayGeneration)
// RDAT: MinShaderTarget: 0x70069

[shader("raygeneration")]
void rg_barrier_reorder_in_call2() {
  fn_barrier_reorder2();
  BAB.Store(0, 0);
}
