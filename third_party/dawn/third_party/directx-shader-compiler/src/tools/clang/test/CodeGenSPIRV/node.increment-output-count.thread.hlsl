// RUN: %dxc -spirv -Od -T lib_6_8 external -fspv-target-env=vulkan1.3 %s | FileCheck %s

// Node with EmptyNodeOutput calls ThreadIncrementOutputCount


[Shader("node")]
[NodeLaunch("thread")]
[NodeIsProgramEntry]
void node028_incrementoutputcount([MaxRecords(32)] EmptyNodeOutput empty)
{
  empty.ThreadIncrementOutputCount(1);
}

// CHECK: [[UINT:%[^ ]*]] = OpTypeInt 32 0
// CHECK-DAG: [[U0:%[^ ]*]] = OpConstant [[UINT]] 0
// CHECK-DAG: [[U1:%[^ ]*]] = OpConstant [[UINT]] 1
// CHECK-DAG: [[STRUCT:%[^ ]*]] = OpTypeStruct
// CHECK-DAG: [[ARR:%[^ ]*]] = OpTypeNodePayloadArrayAMDX [[STRUCT]]
// CHECK-DAG: [[PTR:%[^ ]*]] = OpTypePointer NodePayloadAMDX [[ARR]]
// CHECK-DAG: OpConstantStringAMDX "empty"
// CHECK-DAG: [[U4:%[^ ]*]] = OpConstant [[UINT]] 4
// CHECK: OpAllocateNodePayloadsAMDX [[PTR]] [[U4]] [[U1]] [[U0]]
