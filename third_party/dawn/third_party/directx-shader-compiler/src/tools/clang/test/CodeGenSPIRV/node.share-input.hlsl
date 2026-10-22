// RUN: %dxc -spirv -T lib_6_8 -fspv-target-env=vulkan1.3 %s | FileCheck %s

// Check that the NodeShareInputOf metadata entry is populated correctly

struct entryRecord
{
    int data0;
};

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(2, 1, 1)]
[NumThreads(1, 1, 1)]
void firstNode(DispatchNodeInputRecord<entryRecord> inputData)
{ }

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(2, 1, 1)]
[NumThreads(1, 1, 1)]
[NodeShareInputOf("firstNode")]
void secondNode(DispatchNodeInputRecord<entryRecord> inputData)
{ }

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(2, 1, 1)]
[NumThreads(1, 1, 1)]
[NodeShareInputOf("firstNode", 3)]
void thirdNode(DispatchNodeInputRecord<entryRecord> inputData)
{ }


// CHECK: OpEntryPoint GLCompute %firstNode "firstNode"
// CHECK: OpEntryPoint GLCompute %secondNode "secondNode"
// CHECK: OpEntryPoint GLCompute %thirdNode "thirdNode"
// CHECK-NOT: OpExecutionModeId %firstNode SharesInputWithAMDX
// CHECK: OpExecutionModeId %secondNode SharesInputWithAMDX [[STR:%[^ ]*]] [[U0:%[^ ]*]]
// CHECK: OpExecutionModeId %thirdNode SharesInputWithAMDX [[STR]] [[U3:%[^ ]*]]
// CHECK: [[UINT:%[^ ]*]] = OpTypeInt 32 0
// CHECK-DAG: [[U0:%[^ ]*]] = OpConstant [[UINT]] 0
// CHECK-DAG: [[U3:%[^ ]*]] = OpConstant [[UINT]] 3
