// RUN: %dxc -spirv -Vd -Od -T lib_6_8 -fspv-target-env=vulkan1.3 %s | FileCheck %s
// Note: validation disabled until NodePayloadAMDX pointers are allowed
// as function arguments
// ==================================================================
// Test writing to matrix members of node records
// ==================================================================

// CHECK: OpName [[NODE01:%[^ ]*]] "node01"
// CHECK: OpName [[INPUT1:%[^ ]*]] "input1"
// CHECK: OpName [[NODE02:%[^ ]*]] "node02"
// CHECK: OpName [[INPUT2:%[^ ]*]] "input2"
// CHECK: OpName [[NODE03:%[^ ]*]] "node03"
// CHECK: OpName [[OUTPUT3:%[^ ]*]] "output3"
// CHECK: OpName [[NODE04:%[^ ]*]] "node04"
// CHECK: OpName [[OUTPUTS4:%[^ ]*]] "outputs4"

struct RECORD
{
  row_major float2x2 m0;
  row_major float2x2 m1;
  column_major float2x2 m2;
};

// CHECK-DAG: [[UINT:%[^ ]*]] = OpTypeInt 32 0
// CHECK-DAG: [[U64:%[^ ]*]] = OpConstant [[UINT]] 64
// CHECK-DAG: [[U1:%[^ ]*]] = OpConstant [[UINT]] 1
// CHECK-DAG: [[FLOAT:%[^ ]*]] = OpTypeFloat 32
// CHECK-DAG: [[F111:%[^ ]*]] = OpConstant [[FLOAT]] 111
// CHECK-DAG: [[V2FLOAT:%[^ ]*]] = OpTypeVector [[FLOAT]] 2
// CHECK-DAG: [[C1:%[^ ]*]] = OpConstantComposite [[V2FLOAT]] [[F111]] [[F111]]
// CHECK-DAG: [[MAT2V2FLOAT:[^ ]*]] = OpTypeMatrix [[V2FLOAT]] 2
// CHECK-DAG: [[M1:%[^ ]*]] = OpConstantComposite [[MAT2V2FLOAT]] [[C1]] [[C1]]
// CHECK-DAG: [[INT:%[^ ]*]] = OpTypeInt 32 1
// CHECK-DAG: [[I1:%[^ ]*]] = OpConstant [[INT]] 1
// CHECK-DAG: [[I0:%[^ ]*]] = OpConstant [[INT]] 0
// CHECK-DAG: [[I2:%[^ ]*]] = OpConstant [[INT]] 2
// CHECK-DAG: [[U0:%[^ ]*]] = OpConstant [[UINT]] 0
// CHECK-DAG: [[F222:%[^ ]*]] = OpConstant [[FLOAT]] 222
// CHECK-DAG: [[C2:%[^ ]*]] = OpConstantComposite [[V2FLOAT]] [[F222]] [[F222]]
// CHECK-DAG: [[M2:%[^ ]*]] = OpConstantComposite [[MAT2V2FLOAT]] [[C2]] [[C2]]
// CHECK-DAG: [[U4:%[^ ]*]] = OpConstant [[UINT]] 4
// CHECK-DAG: [[U2:%[^ ]*]] = OpConstant [[UINT]] 2

[Shader("node")]
[NumThreads(1024,1,1)]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(64,1,1)]
void node01(RWDispatchNodeInputRecord<RECORD> input1)
{
  // CHECK: [[NODE01]] = OpFunction
  // CHECK: [[P0:%[^ ]*]] = OpAccessChain %{{[^ ]*}} [[INPUT1]] [[U0]]
  // CHECK: [[P1:%[^ ]*]] = OpAccessChain %{{[^ ]*}} [[P0]] [[I1]]
  // CHECK: OpStore [[P1]] [[M1]]
  // CHECK: [[P0:%[^ ]*]] = OpAccessChain %{{[^ ]*}} [[INPUT1]] [[U0]]
  // CHECK: [[P2:%[^ ]*]] = OpAccessChain %{{[^ ]*}} [[P0]] [[I0]]
  // CHECK: [[VAL:%[^ ]*]] = OpLoad [[MAT2V2FLOAT]] [[P2]]
  // CHECK: [[P0:%[^ ]*]] = OpAccessChain %{{[^ ]*}} [[INPUT1]] [[U0]]
  // CHECK: [[P3:%[^ ]*]] = OpAccessChain %{{[^ ]*}} [[P0]] [[I2]]
  // CHECK: OpStore [[P3]] [[VAL]]
  // CHECK: OpFunctionEnd
  input1.Get().m1 = 111;
  input1.Get().m2 = input1.Get().m0;
}

[Shader("node")]
[NumThreads(1,1,1)]
[NodeLaunch("coalescing")]
void node02([MaxRecords(4)] RWGroupNodeInputRecords<RECORD> input2)
{
  // CHECK: [[NODE02]] = OpFunction
  // CHECK: [[P1:%[^ ]*]] = OpAccessChain %{{[^ ]*}} [[INPUT2]] [[U0]] [[I1]]
  // CHECK: OpStore [[P1]] [[M1]]
  // CHECK: [[P2:%[^ ]*]] = OpAccessChain %{{[^ ]*}} [[INPUT2]] [[U1]] [[I0]]
  // CHECK: [[VAL:%[^ ]*]] = OpLoad [[MAT2V2FLOAT]] [[P2]]
  // CHECK: [[P3:%[^ ]*]] = OpAccessChain %{{[^ ]*}} [[INPUT2]] [[U1]] [[I2]]
  // CHECK: OpStore [[P3]] [[VAL]]
  // CHECK: OpFunctionEnd
  input2[0].m1 = 111;
  input2[1].m2 = input2[1].m0;
}

[Shader("node")]
[NumThreads(1024,1,1)]
[NodeDispatchGrid(64,1,1)]
[NodeLaunch("broadcasting")]
void node03(NodeOutput<RECORD> output3)
{
  // CHECK: [[NODE03]] = OpFunction
  // CHECK: [[PAY:%[^ ]*]] = OpAllocateNodePayloadsAMDX %{{[^ ]*}} [[U4]] [[U1]] [[U0]]
  // CHECK: [[VAL:%[^ ]*]] = OpLoad %{{[^ ]*}} [[PAY]]
  // CHECK: OpStore [[OUTREC3:%[^ ]*]] [[VAL]]
  // CHECK: [[P0:%[^ ]*]] = OpAccessChain %{{[^ ]*}} [[OUTREC3]] [[U0]]
  // CHECK: [[P1:%[^ ]*]] = OpAccessChain %{{[^ ]*}} [[P0]] [[I1]]
  // CHECK: OpStore [[P1]] [[M1]]
  // CHECK: [[P0:%[^ ]*]] = OpAccessChain %{{[^ ]*}} [[OUTREC3]] [[U0]]
  // CHECK: [[P2:%[^ ]*]] = OpAccessChain %{{[^ ]*}} [[P0]] [[I2]]
  // CHECK: OpStore [[P2]] [[M2]]
  // CHECK: OpFunctionEnd
  ThreadNodeOutputRecords<RECORD> outrec = output3.GetThreadNodeOutputRecords(1);
  outrec.Get().m1 = 111;
  outrec.Get().m2 = 222;
}

[Shader("node")]
[NumThreads(1024,1,1)]
[NodeLaunch("coalescing")]
void node04([MaxRecords(5)] NodeOutput<RECORD> outputs4)
{
  // CHECK: [[NODE04]] = OpFunction
  // CHECK: [[PAY:%[^ ]*]] = OpAllocateNodePayloadsAMDX %{{[^ ]*}} [[U2]] [[U1]] [[U0]]
  // CHECK: [[VAL:%[^ ]*]] = OpLoad %{{[^ ]*}} [[PAY]]
  // CHECK: OpStore [[OUTREC4:%[^ ]*]] [[VAL]]
  // CHECK: [[P0:%[^ ]*]] = OpAccessChain %{{[^ ]*}} [[OUTREC4]] [[U0]]
  // CHECK: [[P1:%[^ ]*]] = OpAccessChain %{{[^ ]*}} [[P0]] [[I1]]
  // CHECK: OpStore [[P1]] [[M1]]
  // CHECK: [[P0:%[^ ]*]] = OpAccessChain %{{[^ ]*}} [[OUTREC4]] [[U0]]
  // CHECK: [[P2:%[^ ]*]] = OpAccessChain %{{[^ ]*}} [[P0]] [[I2]]
  // CHECK: OpStore [[P2]] [[M2]]
  // CHECK: OpFunctionEnd
  GroupNodeOutputRecords<RECORD> outrec = outputs4.GetGroupNodeOutputRecords(1);
  outrec.Get().m1 = 111;
  outrec.Get().m2 = 222;
}
