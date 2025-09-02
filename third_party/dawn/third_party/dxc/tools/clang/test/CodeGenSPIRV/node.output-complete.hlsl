// RUN: %dxc -spirv -Od -T lib_6_8 -fspv-target-env=vulkan1.3 %s | FileCheck %s

// OutputComplete() is called with NodeOutput

struct OUTPUT_RECORD
{
  uint value;
};

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(256,1,1)]
[NumThreads(1024,1,1)]
void outputcomplete([MaxRecords(256)] NodeOutput<OUTPUT_RECORD> output)
{
  ThreadNodeOutputRecords<OUTPUT_RECORD> outputrecords = output.GetThreadNodeOutputRecords(1);
    // ...
  outputrecords.OutputComplete();
}

// CHECK: OpName [[RECORDS:%[^ ]*]] "outputrecords"
// CHECK: OpDecorateId [[ARR:%[^ ]*]] PayloadNodeNameAMDX [[STR:%[0-9A-Za-z_]*]]
// CHECK-DAG: [[UINT:%[^ ]*]] = OpTypeInt 32 0
// CHECK-DAG: [[U1:%[^ ]*]] = OpConstant [[UINT]] 1
// CHECK-DAG: [[U0:%[^ ]*]] = OpConstant [[UINT]] 0
// CHECK-DAG: [[REC:%[^ ]*]] = OpTypeStruct [[UINT]]
// CHECK-DAG: [[ARR:%[^ ]*]] = OpTypeNodePayloadArrayAMDX [[REC]]
// CHECK-DAG: [[PTR:%[^ ]*]] = OpTypePointer NodePayloadAMDX [[ARR]]
// CHECK-DAG: [[U4:[^ ]*]] = OpConstant [[UINT]] 4
// CHECK: [[V0:%[^ ]*]] = OpAllocateNodePayloadsAMDX [[PTR]] [[U4]] [[U1]] [[U0]]
// CHECK: [[V1:%[^ ]*]] = OpLoad [[ARR]] [[V0]]
// CHECK: OpStore [[RECORDS]] [[V1]]
// CHECK: OpEnqueueNodePayloadsAMDX [[RECORDS]]
