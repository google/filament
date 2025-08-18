// RUN: %dxc -spirv -Vd -Od -T lib_6_8 -fspv-target-env=vulkan1.3 -enable-16bit-types %s | FileCheck %s
// Note: validation disabled until NodePayloadAMDX pointers are allowed
// as function arguments

// Read access of members of input/output record with different type
// sizes - we check the function specializations generated

RWBuffer<uint> buf0;

struct RECORD
{
  half h;
  float f;
  double d;
  bool b;
  uint16_t i16;
  int i;
  int64_t i64;
  uint64_t u64;
};

// CHECK: OpName [[BUF0:%[^ ]*]] "buf0"
// CHECK-DAG: OpName [[RECORD:%[^ ]*]] "RECORD"
// CHECK-DAG: OpMemberName [[RECORD]] 0 "h"
// CHECK-DAG: OpMemberName [[RECORD]] 1 "f"
// CHECK-DAG: OpMemberName [[RECORD]] 2 "d"
// CHECK-DAG: OpMemberName [[RECORD]] 3 "b"
// CHECK-DAG: OpMemberName [[RECORD]] 4 "i16"
// CHECK-DAG: OpMemberName [[RECORD]] 5 "i"
// CHECK-DAG: OpMemberName [[RECORD]] 6 "i64"
// CHECK-DAG: OpMemberName [[RECORD]] 7 "u64"

// CHECK-DAG: [[UINT:%[^ ]*]] = OpTypeInt 32 0
// CHECK-DAG: [[INT:%[^ ]*]] = OpTypeInt 32 1
// CHECK-DAG: [[S0:%[^ ]*]] = OpConstant [[INT]] 0
// CHECK-DAG: [[S1:%[^ ]*]] = OpConstant [[INT]] 1
// CHECK-DAG: [[S2:%[^ ]*]] = OpConstant [[INT]] 2
// CHECK-DAG: [[S3:%[^ ]*]] = OpConstant [[INT]] 3
// CHECK-DAG: [[S4:%[^ ]*]] = OpConstant [[INT]] 4
// CHECK-DAG: [[S5:%[^ ]*]] = OpConstant [[INT]] 5
// CHECK-DAG: [[S6:%[^ ]*]] = OpConstant [[INT]] 6
// CHECK-DAG: [[S7:%[^ ]*]] = OpConstant [[INT]] 7
// CHECK-DAG: [[U0:%[^ ]*]] = OpConstant [[UINT]] 0
// CHECK-DAG: [[U1:%[^ ]*]] = OpConstant [[UINT]] 1
// CHECK-DAG: [[TBI:%[^ ]*]] = OpTypeImage [[UINT]] Buffer

// CHECK-DAG: [[HALF:%[^ ]*]] = OpTypeFloat 16
// CHECK-DAG: [[FLOAT:%[^ ]*]] = OpTypeFloat 32
// CHECK-DAG: [[DOUBLE:%[^ ]*]] = OpTypeFloat 64
// CHECK-DAG: [[USHORT:%[^ ]*]] = OpTypeInt 16 0
// CHECK-DAG: [[LONG:%[^ ]*]] = OpTypeInt 64 1
// CHECK-DAG: [[ULONG:%[^ ]*]] = OpTypeInt 64 0
// CHECK: [[RECORD]] = OpTypeStruct [[HALF]] [[FLOAT]] [[DOUBLE]] [[UINT]] [[USHORT]] [[INT]] [[LONG]] [[ULONG]]
// CHECK: [[BOOL:%[^ ]*]] = OpTypeBool

[Shader("node")]
[NumThreads(1024,1,1)]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(64,1,1)]
void node01(DispatchNodeInputRecord<RECORD> input)
{
  buf0[0] = input.Get().h;
}

// CHECK: OpFunction
// CHECK: [[PTR:%[^ ]*]] = OpAccessChain %{{[^ ]*}} %{{[^ ]*}} [[S0]]
// CHECK: [[VAL0:%[^ ]*]] = OpLoad [[HALF]] [[PTR]]
// CHECK: [[VAL1:%[^ ]*]] = OpConvertFToU [[UINT]] [[VAL0]]
// CHECK: [[VAL2:%[^ ]*]] = OpLoad [[TBI]] [[BUF0]]
// CHECK: OpImageWrite [[VAL2]] [[U0]] [[VAL1]] None
// CHECK: OpFunctionEnd

[Shader("node")]
[NumThreads(1024,1,1)]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(64,1,1)]
void node02(DispatchNodeInputRecord<RECORD> input)
{
  buf0[0] = input.Get().f;
}

// CHECK: OpFunction
// CHECK: [[PTR:%[^ ]*]] = OpAccessChain %{{[^ ]*}} %{{[^ ]*}} [[S1]]
// CHECK: [[VAL0:%[^ ]*]] = OpLoad [[FLOAT]] [[PTR]]
// CHECK: [[VAL1:%[^ ]*]] = OpConvertFToU [[UINT]] [[VAL0]]
// CHECK: [[VAL2:%[^ ]*]] = OpLoad [[TBI]] [[BUF0]]
// CHECK: OpImageWrite [[VAL2]] [[U0]] [[VAL1]] None
// CHECK: OpFunctionEnd

[Shader("node")]
[NumThreads(1024,1,1)]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(64,1,1)]
void node03(DispatchNodeInputRecord<RECORD> input)
{
  buf0[0] = input.Get().d;
}

// CHECK: OpFunction
// CHECK: [[PTR:%[^ ]*]] = OpAccessChain %{{[^ ]*}} %{{[^ ]*}} [[S2]]
// CHECK: [[VAL0:%[^ ]*]] = OpLoad [[DOUBLE]] [[PTR]]
// CHECK: [[VAL1:%[^ ]*]] = OpConvertFToU [[UINT]] [[VAL0]]
// CHECK: [[VAL2:%[^ ]*]] = OpLoad [[TBI]] [[BUF0]]
// CHECK: OpImageWrite [[VAL2]] [[U0]] [[VAL1]] None
// CHECK: OpFunctionEnd

[Shader("node")]
[NumThreads(1024,1,1)]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(64,1,1)]
void node04(DispatchNodeInputRecord<RECORD> input)
{
  buf0[0] = input.Get().b;
}

// CHECK: OpFunction
// CHECK: [[PTR:%[^ ]*]] = OpAccessChain %{{[^ ]*}} %{{[^ ]*}} [[S3]]
// CHECK: [[VAL0:%[^ ]*]] = OpLoad [[UINT]] [[PTR]]
// CHECK: [[VAL1:%[^ ]*]] = OpINotEqual [[BOOL]] [[VAL0]] [[U0]]
// CHECK: [[VAL2:%[^ ]*]] = OpSelect [[UINT]] [[VAL1]] [[U1]] [[U0]]
// CHECK: [[VAL3:%[^ ]*]] = OpLoad [[TBI]] [[BUF0]]
// CHECK: OpImageWrite [[VAL3]] [[U0]] [[VAL2]] None
// CHECK: OpFunctionEnd

[Shader("node")]
[NumThreads(1024,1,1)]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(64,1,1)]
void node05(DispatchNodeInputRecord<RECORD> input)
{
  buf0[0] = input.Get().i16;
}

// CHECK: OpFunction
// CHECK: [[PTR:%[^ ]*]] = OpAccessChain %{{[^ ]*}} %{{[^ ]*}} [[S4]]
// CHECK: [[VAL0:%[^ ]*]] = OpLoad [[USHORT]] [[PTR]]
// CHECK: [[VAL1:%[^ ]*]] = OpUConvert [[UINT]] [[VAL0]]
// CHECK: [[VAL2:%[^ ]*]] = OpLoad [[TBI]] [[BUF0]]
// CHECK: OpImageWrite [[VAL2]] [[U0]] [[VAL1]] None
// CHECK: OpFunctionEnd

[Shader("node")]
[NumThreads(1024,1,1)]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(64,1,1)]
void node06(DispatchNodeInputRecord<RECORD> input)
{
  buf0[0] = input.Get().i;
}

// CHECK: OpFunction
// CHECK: [[PTR:%[^ ]*]] = OpAccessChain %{{[^ ]*}} %{{[^ ]*}} [[S5]]
// CHECK: [[VAL0:%[^ ]*]] = OpLoad [[INT]] [[PTR]]
// CHECK: [[VAL1:%[^ ]*]] = OpBitcast [[UINT]] [[VAL0]]
// CHECK: [[VAL2:%[^ ]*]] = OpLoad [[TBI]] [[BUF0]]
// CHECK: OpImageWrite [[VAL2]] [[U0]] [[VAL1]] None
// CHECK: OpFunctionEnd

[Shader("node")]
[NumThreads(1024,1,1)]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(64,1,1)]
void node07(DispatchNodeInputRecord<RECORD> input)
{
  buf0[0] = input.Get().i64;
}

// CHECK: OpFunction
// CHECK: [[PTR:%[^ ]*]] = OpAccessChain %{{[^ ]*}} %{{[^ ]*}} [[S6]]
// CHECK: [[VAL0:%[^ ]*]] = OpLoad [[LONG]] [[PTR]]
// CHECK: [[VAL1:%[^ ]*]] = OpSConvert [[INT]] [[VAL0]]
// CHECK: [[VAL2:%[^ ]*]] = OpBitcast [[UINT]] [[VAL1]]
// CHECK: [[VAL3:%[^ ]*]] = OpLoad [[TBI]] [[BUF0]]
// CHECK: OpImageWrite [[VAL3]] [[U0]] [[VAL2]] None
// CHECK: OpFunctionEnd

[Shader("node")]
[NumThreads(1024,1,1)]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(64,1,1)]
void node08(DispatchNodeInputRecord<RECORD> input)
{
  buf0[0] = input.Get().u64;
}

// CHECK: OpFunction
// CHECK: [[PTR:%[^ ]*]] = OpAccessChain %{{[^ ]*}} %{{[^ ]*}} [[S7]]
// CHECK: [[VAL0:%[^ ]*]] = OpLoad [[ULONG]] [[PTR]]
// CHECK: [[VAL1:%[^ ]*]] = OpUConvert [[UINT]] [[VAL0]]
// CHECK: [[VAL2:%[^ ]*]] = OpLoad [[TBI]] [[BUF0]]
// CHECK: OpImageWrite [[VAL2]] [[U0]] [[VAL1]] None
// CHECK: OpFunctionEnd

