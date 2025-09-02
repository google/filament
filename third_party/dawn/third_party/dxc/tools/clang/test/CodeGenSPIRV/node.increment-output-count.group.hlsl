// RUN: %dxc -spirv -Od -T lib_6_8 external -fspv-target-env=vulkan1.3 %s | FileCheck %s

// Node with EmptyNodeOutput calls GroupIncrementOutputCount


[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(1024,1,1)]
[NodeIsProgramEntry]
void node028_incrementoutputcount([MaxRecords(32)] EmptyNodeOutput empty)
{
  empty.GroupIncrementOutputCount(1);
}

// CHECK: [[UINT:%[^ ]*]] = OpTypeInt 32 0
// CHECK-DAG: [[U0:%[^ ]*]] = OpConstant [[UINT]] 0
// CHECK-DAG: [[U1:%[^ ]*]] = OpConstant [[UINT]] 1
// CHECK-DAG: [[STRUCT:%[^ ]*]] = OpTypeStruct
// CHECK-DAG: [[ARR:%[^ ]*]] = OpTypeNodePayloadArrayAMDX [[STRUCT]]
// CHECK-DAG: [[PTR:%[^ ]*]] = OpTypePointer NodePayloadAMDX [[ARR]]
// CHECK-DAG: [[U2:%[^ ]*]] = OpConstant [[UINT]] 2
// CHECK: OpAllocateNodePayloadsAMDX [[PTR]] [[U2]] [[U1]] [[U0]]
