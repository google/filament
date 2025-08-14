// RUN: %dxc -spirv -Vd -Od -T lib_6_8 -fspv-target-env=vulkan1.3 %s | FileCheck %s
// Note: validation disabled until NodePayloadAMDX pointers are allowed
// as function arguments

// Barrier is called with each node record and UAV type

struct RECORD
{
    uint value;
};

// CHECK: [[UINT:%[^ ]*]] = OpTypeInt 32 0
// CHECK-DAG: [[U256:%[^ ]*]] = OpConstant [[UINT]] 256
// CHECK-DAG: [[U1:%[^ ]*]] = OpConstant [[UINT]] 1
// CHECK-DAG: [[U0:%[^ ]*]] = OpConstant [[UINT]] 0
// CHECK-DAG: [[U3:%[^ ]*]] = OpConstant [[UINT]] 3
// CHECK-DAG: [[U4:%[^ ]*]] = OpConstant [[UINT]] 4
// CHECK-DAG: [[U2:%[^ ]*]] = OpConstant [[UINT]] 2
// CHECK-DAG: [[U4424:%[^ ]*]] = OpConstant [[UINT]] 4424
// CHECK-DAG: [[U5:%[^ ]*]] = OpConstant [[UINT]] 5

[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(256,1,1)]
[NodeDispatchGrid(256,1,1)]
void node01(DispatchNodeInputRecord<RECORD> input)
{
   Barrier(input, 5);
}

// CHECK: OpControlBarrier %uint_2 %uint_5 %uint_4424

[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(256,1,1)]
void node02([MaxRecords(8)] GroupNodeInputRecords<RECORD> input)
{
   Barrier(input, 3);
}

// CHECK: OpControlBarrier %uint_2 %uint_2 %uint_4424

[Shader("node")]
[NodeLaunch("thread")]
void node03(RWThreadNodeInputRecord<RECORD> input)
{
   Barrier(input, 0);
}

// CHECK: OpMemoryBarrier %uint_4 %uint_4424

[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(256,1,1)]
void node04([MaxRecords(6)] RWGroupNodeInputRecords<RECORD> input)
{
   Barrier(input, 0);
}

// CHECK: OpMemoryBarrier %uint_4 %uint_4424

[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(256,1,1)]
[NodeDispatchGrid(256,1,1)]
void node05([MaxRecords(5)] NodeOutput<RECORD> outputs)
{
   ThreadNodeOutputRecords<RECORD> outrec = outputs.GetThreadNodeOutputRecords(1);
   Barrier(outrec, 0);
}

// CHECK: OpMemoryBarrier %uint_4 %uint_4424

[Shader("node")]
[NodeLaunch("thread")]
void node06([MaxRecords(5)] NodeOutput<RECORD> outputs)
{
   ThreadNodeOutputRecords<RECORD> outrec = outputs.GetThreadNodeOutputRecords(3);
   Barrier(outrec, 0);
}

// CHECK: OpMemoryBarrier %uint_4 %uint_4424

[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(256,1,3)]
void node07([MaxRecords(5)] NodeOutput<RECORD> outputs)
{
   GroupNodeOutputRecords<RECORD> outrec = outputs.GetGroupNodeOutputRecords(1);
   Barrier(outrec, 3);
}

// CHECK: OpControlBarrier %uint_2 %uint_2 %uint_4424

[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(256,1,4)]
[NodeDispatchGrid(256,1,1)]
void node08([MaxRecords(5)] NodeOutput<RECORD> outputs)
{
   GroupNodeOutputRecords<RECORD> outrec = outputs.GetGroupNodeOutputRecords(4);
   Barrier(outrec, 3);
}

// CHECK: OpControlBarrier %uint_2 %uint_2 %uint_4424

RWBuffer<float> obj09;
[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(256,1,4)]
[NodeDispatchGrid(256,1,1)]
void node09()
{
   Barrier(obj09, 5);
}

// CHECK: OpControlBarrier %uint_2 %uint_5 %uint_4424

RWTexture1D<float4> obj10;
[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(256,1,4)]
[NodeDispatchGrid(256,1,1)]
void node10()
{
   Barrier(obj10, 5);
}

// CHECK: OpControlBarrier %uint_2 %uint_5 %uint_4424

RWTexture1DArray<float4> obj11;
[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(256,1,4)]
[NodeDispatchGrid(256,1,1)]
void node11()
{
   Barrier(obj11, 5);
}

// CHECK: OpControlBarrier %uint_2 %uint_5 %uint_4424

RWTexture2D<float> obj12;
[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(256,1,4)]
[NodeDispatchGrid(256,1,1)]
void node12()
{
   Barrier(obj12, 5);
}

// CHECK: OpControlBarrier %uint_2 %uint_5 %uint_4424

RWTexture2DArray<float> obj13;
[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(256,1,4)]
[NodeDispatchGrid(256,1,1)]
void node13()
{
   Barrier(obj13, 5);
}

// CHECK: OpControlBarrier %uint_2 %uint_5 %uint_4424

RWTexture3D<float> obj14;
[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(256,1,4)]
[NodeDispatchGrid(256,1,1)]
void node14()
{
   Barrier(obj14, 5);
}

// CHECK: OpControlBarrier %uint_2 %uint_5 %uint_4424

RWStructuredBuffer<RECORD> obj15;
[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(256,1,4)]
[NodeDispatchGrid(256,1,1)]
void node15()
{
   Barrier(obj15, 5);
}

// CHECK: OpControlBarrier %uint_2 %uint_5 %uint_4424

RWByteAddressBuffer obj16;
[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(256,1,4)]
[NodeDispatchGrid(256,1,1)]
void node16()
{
   Barrier(obj16, 5);
}

// CHECK: OpControlBarrier %uint_2 %uint_5 %uint_4424

AppendStructuredBuffer<RECORD> obj17;
[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(256,1,4)]
[NodeDispatchGrid(256,1,1)]
void node17()
{
   Barrier(obj17, 5);
}

// CHECK: OpControlBarrier %uint_2 %uint_5 %uint_4424
