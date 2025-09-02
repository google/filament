// RUN: %dxc -spirv -Od -T lib_6_8 -fspv-target-env=vulkan1.3 %s | FileCheck %s

// Multiple calls to Get*NodeOuputRecords(array)

struct RECORD {
  int i;
  float3 foo;
};

[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(64, 1, 1)]
[NodeDispatchGrid(8, 1, 1)]
void node150_a(NodeOutput<RECORD> output)
{
  GroupNodeOutputRecords<RECORD> outRec1 = output.GetGroupNodeOutputRecords(1);
  GroupNodeOutputRecords<RECORD> outRec2 = output.GetGroupNodeOutputRecords(4);
  outRec1.OutputComplete();
  outRec2.OutputComplete();
}

[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(64, 1, 1)]
[NodeDispatchGrid(8, 1, 1)]
void node150_b(NodeOutput<RECORD> output)
{
  ThreadNodeOutputRecords<RECORD> outRec1 = output.GetThreadNodeOutputRecords(5);
  ThreadNodeOutputRecords<RECORD> outRec2 = output.GetThreadNodeOutputRecords(1);
  outRec1.OutputComplete();
  outRec1 = outRec2;
  outRec1.OutputComplete();
}

// CHECK: OpDecorateId [[ARR_A:%[^ ]*]] PayloadNodeNameAMDX [[STR:%[0-9A-Za-z_]*]]
// CHECK: OpDecorateId [[ARR_B:%[^ ]*]] PayloadNodeNameAMDX [[STR]]

// CHECK: [[UINT:%[^ ]*]] = OpTypeInt 32 0
// CHECK-DAG: [[U0:%[^ ]*]] = OpConstant [[UINT]] 0
// CHECK-DAG: [[U1:%[^ ]*]] = OpConstant [[UINT]] 1
// CHECK-DAG: [[U2:%[^ ]*]] = OpConstant [[UINT]] 2
// CHECK-DAG: [[U4:%[^ ]*]] = OpConstant [[UINT]] 4
// CHECK-DAG: [[U5:%[^ ]*]] = OpConstant [[UINT]] 5
// CHECK-DAG: [[STR]] = OpConstantStringAMDX "output"
// CHECK-DAG: [[ARR_A]] = OpTypeNodePayloadArrayAMDX
// CHECK-DAG: [[ARR_B]] = OpTypeNodePayloadArrayAMDX
// CHECK-DAG: [[FPTR_A:%[^ ]*]] = OpTypePointer Function [[ARR_A]]
// CHECK-DAG: [[NPTR_A:%[^ ]*]] = OpTypePointer NodePayloadAMDX [[ARR_A]]
// CHECK-DAG: [[FPTR_B:%[^ ]*]] = OpTypePointer Function [[ARR_B]]
// CHECK-DAG: [[NPTR_B:%[^ ]*]] = OpTypePointer NodePayloadAMDX [[ARR_B]]

// checking for OpFunctionCall skips over the entry function wrapper and
// thereby avoids matching wrapper variables
// CHECK: OpFunctionCall
// CHECK: [[OUT1:%[^ ]*]] = OpVariable [[FPTR_A]]
// CHECK: [[OUT2:%[^ ]*]] = OpVariable [[FPTR_A]]
// CHECK: [[PAY:%[^ ]*]] = OpAllocateNodePayloadsAMDX [[NPTR_A]] [[U2]] [[U1]] [[U0]]
// CHECK: [[VAL:%[^ ]*]] = OpLoad [[ARR_A]] [[PAY]]
// CHECK: OpStore [[OUT1]] [[VAL]]
// CHECK: [[PAY:%[^ ]*]] = OpAllocateNodePayloadsAMDX [[NPTR_A]] [[U2]] [[U4]] [[U0]]
// CHECK: [[VAL:%[^ ]*]] = OpLoad [[ARR_A]] [[PAY]]
// CHECK: OpStore [[OUT2]] [[VAL]]
// CHECK: OpFunctionCall
// CHECK: [[OUT1:%[^ ]*]] = OpVariable [[FPTR_B]]
// CHECK: [[OUT2:%[^ ]*]] = OpVariable [[FPTR_B]]
// CHECK: [[PAY:%[^ ]*]] = OpAllocateNodePayloadsAMDX [[NPTR_B]] [[U4]] [[U5]] [[U0]]
// CHECK: [[VAL:%[^ ]*]] = OpLoad [[ARR_B]] [[PAY]]
// CHECK: OpStore [[OUT1]] [[VAL]]
// CHECK: [[PAY:%[^ ]*]] = OpAllocateNodePayloadsAMDX [[NPTR_B]] [[U4]] [[U1]] [[U0]]
// CHECK: [[VAL:%[^ ]*]] = OpLoad [[ARR_B]] [[PAY]]
// CHECK: OpStore [[OUT2]] [[VAL]]
// CHECK: OpFunctionEnd
