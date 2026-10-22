// RUN: %dxc -spirv -T lib_6_8 external -fspv-target-env=vulkan1.3 %s | FileCheck %s

// GetRemainingRecusionLevels() called

RWBuffer<uint> buf0;

[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(8,1,1)]
[NodeDispatchGrid(32,2,2)]
[NodeMaxRecursionDepth(16)]
void node133_getremainingrecursionlevels()
{
  uint remaining = GetRemainingRecursionLevels();
  // Use resource as a way of preventing DCE
  buf0[0] = remaining;
}

// CHECK: OpEntryPoint GLCompute [[SHADER:%[^ ]*]] "node133_getremainingrecursionlevels" [[RRL:%[^ ]*]]
// CHECK: OpExecutionModeId [[SHADER]] MaxNodeRecursionAMDX [[U16:%[^ ]*]]
// CHECK: OpDecorate [[RRL]] BuiltIn RemainingRecursionLevelsAMDX
// CHECK: [[UINT:%[^ ]*]] = OpTypeInt 32 0
// CHECK: [[U16]] = OpConstant [[UINT]] 16
// CHECK: [[PTR:%[^ ]*]] = OpTypePointer Input [[UINT]]
// CHECK: [[RRL]] = OpVariable [[PTR]] Input
// CHECK: OpLoad [[UINT]] [[RRL]]
