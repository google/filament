// RUN: %dxc -spirv -Vd -Od -T lib_6_8 -fspv-target-env=vulkan1.3 %s | FileCheck %s
// Note: validation disabled until NodePayloadAMDX pointers are allowed
// as function arguments

// Read access to members of node input/output records

RWBuffer<uint> buf0;

struct RECORD
{
  uint a;
  uint b;
  uint c;
};

// CHECK: OpName [[BUF0:%[^ ]*]] "buf0"
// CHECK: [[UINT:%[^ ]*]] = OpTypeInt 32 0
// CHECK: [[U0:%[^ ]*]] = OpConstant [[UINT]] 0
// CHECK: [[U16:%[^ ]*]] = OpConstant [[UINT]] 16
// CHECK-DAG: [[INT:%[^ ]*]] = OpTypeInt 32 1
// CHECK-DAG: [[S0:%[^ ]*]] = OpConstant [[INT]] 0
// CHECK-DAG: [[U1:%[^ ]*]] = OpConstant [[UINT]] 1
// CHECK-DAG: [[S1:%[^ ]*]] = OpConstant [[INT]] 1
// CHECK-DAG: [[U2:%[^ ]*]] = OpConstant [[UINT]] 2
// CHECK-DAG: [[S2:%[^ ]*]] = OpConstant [[INT]] 2
// CHECK-DAG: [[U4:%[^ ]*]] = OpConstant [[UINT]] 4
// CHECK-DAG: [[U7:%[^ ]*]] = OpConstant [[UINT]] 7
// CHECK-DAG: [[TBI:%[^ ]*]] = OpTypeImage [[UINT]] Buffer

[Shader("node")]
[NumThreads(1024,1,1)]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(16,1,1)]
void node01(DispatchNodeInputRecord<RECORD> input)
{
  buf0[0] = input.Get().a;
}

// CHECK: OpFunction
// CHECK: [[PTR:%[^ ]*]] = OpAccessChain %{{[^ ]*}} %{{[^ ]*}} [[S0]]
// CHECK: [[VAL:%[^ ]*]] = OpLoad [[UINT]] [[PTR]]
// CHECK: [[IMG:%[^ ]*]] = OpLoad [[TBI]] [[BUF0]]
// CHECK: OpImageWrite [[IMG]] [[U0]] [[VAL]]
// CHECK: OpFunctionEnd


[Shader("node")]
[NumThreads(1024,1,1)]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(16,1,1)]
void node02(RWDispatchNodeInputRecord<RECORD> input)
{
  buf0[0] = input.Get().b;
}

// CHECK: OpFunction
// CHECK: [[PTR:%[^ ]*]] = OpAccessChain %{{[^ ]*}} %{{[^ ]*}} [[S1]]
// CHECK: [[VAL:%[^ ]*]] = OpLoad [[UINT]] [[PTR]]
// CHECK: [[IMG:%[^ ]*]] = OpLoad [[TBI]] [[BUF0]]
// CHECK: OpImageWrite [[IMG]] [[U0]] [[VAL]]
// CHECK: OpFunctionEnd

[Shader("node")]
[NumThreads(1024, 1, 1)]
[NodeLaunch("coalescing")]
void node03([MaxRecords(3)] GroupNodeInputRecords<RECORD> input)
{
  buf0[0] = input[1].c;
}

// CHECK: OpFunction
// CHECK: [[PTR:%[^ ]*]] = OpAccessChain %{{[^ ]*}} %{{[^ ]*}} [[U1]] [[S2]]
// CHECK: [[VAL:%[^ ]*]] = OpLoad [[UINT]] [[PTR]]
// CHECK: [[IMG:%[^ ]*]] = OpLoad [[TBI]] [[BUF0]]
// CHECK: OpImageWrite [[IMG]] [[U0]] [[VAL]]
// CHECK: OpFunctionEnd

[Shader("node")]
[NumThreads(1,1,1)]
[NodeLaunch("coalescing")]
void node04([MaxRecords(4)] RWGroupNodeInputRecords<RECORD> input)
{
  buf0[0] = input[2].c;
}

// CHECK: OpFunction
// CHECK: [[PTR:%[^ ]*]] = OpAccessChain %{{[^ ]*}} %{{[^ ]*}} [[U2]] [[S2]]
// CHECK: [[VAL:%[^ ]*]] = OpLoad [[UINT]] [[PTR]]
// CHECK: [[IMG:%[^ ]*]] = OpLoad [[TBI]] [[BUF0]]
// CHECK: OpImageWrite [[IMG]] [[U0]] [[VAL]]
// CHECK: OpFunctionEnd

[Shader("node")]
[NumThreads(1,1,1)]
[NodeLaunch("coalescing")]
void node05(NodeOutput<RECORD> output)
{
  ThreadNodeOutputRecords<RECORD> outrec = output.GetThreadNodeOutputRecords(1);
  buf0[0] = outrec.Get().a;
}

// CHECK: OpFunction
// CHECK: [[PAY:%[^ ]*]] = OpAllocateNodePayloadsAMDX %{{[^ ]*}} [[U4]] [[U1]] [[U0]]
// CHECK: [[TEMP:%[^ ]*]] = OpLoad %{{[^ ]*}} [[PAY]]
// CHECK: OpStore [[OUT:%[^ ]*]] [[TEMP]]
// CHECK: [[PTR1:%[^ ]*]] = OpAccessChain %{{[^ ]*}} [[OUT]] [[U0]]
// CHECK: [[PTR2:%[^ ]*]] = OpAccessChain %{{[^ ]*}} [[PTR1]] [[S0]]
// CHECK-DAG: [[VAL:%[^ ]*]] = OpLoad [[UINT]] [[PTR2]]
// CHECK-DAG: [[IMG:%[^ ]*]] = OpLoad [[TBI]] [[BUF0]]
// CHECK: OpImageWrite [[IMG]] [[U0]] [[VAL]]
// CHECK: OpFunctionEnd

[Shader("node")]
[NumThreads(1,1,1)]
[NodeLaunch("coalescing")]
void node06(NodeOutput<RECORD> output)
{
  ThreadNodeOutputRecords<RECORD> outrec = output.GetThreadNodeOutputRecords(7);
  buf0[0] = outrec[2].b;
}

// CHECK: OpFunction
// CHECK: [[PAY:%[^ ]*]] = OpAllocateNodePayloadsAMDX %{{[^ ]*}} [[U4]] [[U7]] [[U0]]
// CHECK: [[TEMP:%[^ ]*]] = OpLoad %{{[^ ]*}} [[PAY]]
// CHECK: OpStore [[OUT:%[^ ]*]] [[TEMP]]
// CHECK: [[PTR:%[^ ]*]] = OpAccessChain %{{[^ ]*}} [[OUT]] [[U2]] [[S1]]
// CHECK-DAG: [[VAL:%[^ ]*]] = OpLoad [[UINT]] [[PTR]]
// CHECK-DAG: [[IMG:%[^ ]*]] = OpLoad [[TBI]] [[BUF0]]
// CHECK: OpImageWrite [[IMG]] [[U0]] [[VAL]]
// CHECK: OpFunctionEnd

[Shader("node")]
[NumThreads(1,1,1)]
[NodeLaunch("coalescing")]
void node07(NodeOutput<RECORD> output)
{
  GroupNodeOutputRecords<RECORD> outrec = output.GetGroupNodeOutputRecords(1);
  buf0[0] = outrec.Get().c;
}

// CHECK: OpFunction
// CHECK: [[PAY:%[^ ]*]] = OpAllocateNodePayloadsAMDX %{{[^ ]*}} [[U2]] [[U1]] [[U0]]
// CHECK: [[TEMP:%[^ ]*]] = OpLoad %{{[^ ]*}} [[PAY]]
// CHECK: OpStore [[OUT:%[^ ]*]] [[TEMP]]
// CHECK: [[PTR1:%[^ ]*]] = OpAccessChain %{{[^ ]*}} [[OUT]] [[U0]]
// CHECK: [[PTR2:%[^ ]*]] = OpAccessChain %{{[^ ]*}} [[PTR1]] [[S2]]
// CHECK-DAG: [[VAL:%[^ ]*]] = OpLoad [[UINT]] [[PTR2]]
// CHECK-DAG: [[IMG:%[^ ]*]] = OpLoad [[TBI]] [[BUF0]]
// CHECK: OpImageWrite [[IMG]] [[U0]] [[VAL]]
// CHECK: OpFunctionEnd
