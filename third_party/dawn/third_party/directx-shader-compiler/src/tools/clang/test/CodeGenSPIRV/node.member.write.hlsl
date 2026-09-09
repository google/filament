// RUN: %dxc -spirv -Vd -Od -T lib_6_8 -fspv-target-env=vulkan1.3 %s | FileCheck %s
// Note: validation disabled until NodePayloadAMDX pointers are allowed
// as function arguments

// Writes to members of the various read-write node records

struct RECORD
{
  uint a;
  uint b;
};

// CHECK-DAG: [[UINT:%[^ ]*]] = OpTypeInt 32 0
// CHECK-DAG: [[INT:%[^ ]*]] = OpTypeInt 32 1
// CHECK-DAG: [[U0:%[^ ]*]] = OpConstant [[UINT]] 0
// CHECK-DAG: [[S0:%[^ ]*]] = OpConstant [[INT]] 0
// CHECK-DAG: [[U1:%[^ ]*]] = OpConstant [[UINT]] 1
// CHECK-DAG: [[S1:%[^ ]*]] = OpConstant [[INT]] 1
// CHECK-DAG: [[U2:%[^ ]*]] = OpConstant [[UINT]] 2
// CHECK-DAG: [[U4:%[^ ]*]] = OpConstant [[UINT]] 4
// CHECK-DAG: [[U5:%[^ ]*]] = OpConstant [[UINT]] 5
// CHECK-DAG: [[U7:%[^ ]*]] = OpConstant [[UINT]] 7
// CHECK-DAG: [[U8:%[^ ]*]] = OpConstant [[UINT]] 8
// CHECK-DAG: [[U9:%[^ ]*]] = OpConstant [[UINT]] 9
// CHECK-DAG: [[U11:%[^ ]*]] = OpConstant [[UINT]] 11

[Shader("node")]
[NumThreads(1024,1,1)]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(64,1,1)]
void node01(RWDispatchNodeInputRecord<RECORD> input1)
{
  input1.Get().a = 5;
}

// CHECK: OpFunction
// CHECK: [[PTR:%[^ ]*]] = OpAccessChain %{{[^ ]*}} %{{[^ ]*}} [[S0]]
// CHECK: OpStore [[PTR]] [[U5]]
// CHECK: OpFunctionEnd

[Shader("node")]
[NumThreads(2,1,1)]
[NodeLaunch("coalescing")]
void node02([MaxRecords(4)] RWGroupNodeInputRecords<RECORD> input2)
{
  input2[1].b = 7;
}

// CHECK: OpFunction
// CHECK: [[PTR:%[^ ]*]] = OpAccessChain %{{[^ ]*}} %{{[^ ]*}} [[U1]] [[S1]]
// CHECK: OpStore [[PTR]] [[U7]]
// CHECK: OpFunctionEnd

[Shader("node")]
[NumThreads(3,1,1)]
[NodeLaunch("coalescing")]
void node03(NodeOutput<RECORD> output)
{
  ThreadNodeOutputRecords<RECORD> output3 = output.GetThreadNodeOutputRecords(2);
  output3.Get().b = 9;
}

// CHECK: OpFunction
// CHECK: [[PAY:%[^ ]*]] = OpAllocateNodePayloadsAMDX %{{[^ ]*}} [[U4]] [[U2]] [[U0]]
// CHECK: [[VAL:%[^ ]*]] = OpLoad %{{[^ ]*}} [[PAY]]
// CHECK: OpStore [[OUT:%[^ ]*]] [[VAL]]
// CHECK: [[PTR0:%[^ ]*]] = OpAccessChain %{{[^ ]*}} [[OUT]] [[U0]]
// CHECK: [[PTR1:%[^ ]*]] = OpAccessChain %{{[^ ]*}} [[PTR0]] [[S1]]
// CHECK: OpStore [[PTR1]] [[U9]]
// CHECK: OpFunctionEnd

[Shader("node")]
[NumThreads(4,1,1)]
[NodeLaunch("coalescing")]
void node04(NodeOutput<RECORD> output)
{
  GroupNodeOutputRecords<RECORD> output4 = output.GetGroupNodeOutputRecords(8);
  output4[0].a = 11;
}

// CHECK: OpFunction
// CHECK: [[PAY:%[^ ]*]] = OpAllocateNodePayloadsAMDX %{{[^ ]*}} [[U2]] [[U8]] [[U0]]
// CHECK: [[VAL:%[^ ]*]] = OpLoad %{{[^ ]*}} [[PAY]]
// CHECK: OpStore [[OUT:%[^ ]*]] [[VAL]]
// CHECK: [[PTR:%[^ ]*]] = OpAccessChain %{{[^ ]*}} [[OUT]] [[U0]] [[S0]]
// CHECK: OpStore [[PTR]] [[U11]]
// CHECK: OpFunctionEnd

