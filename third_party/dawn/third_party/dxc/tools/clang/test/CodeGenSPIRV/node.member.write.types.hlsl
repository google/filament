// RUN: %dxc -spirv -T lib_6_8 -fspv-target-env=vulkan1.3 -enable-16bit-types %s | FileCheck %s

// Writes to node record members of various types


struct RECORD
{
  half h;
  float f;
  double d;
  bool b;
  int16_t i16;
  uint16_t u16;
  int i;
  int64_t i64;
  uint64_t u64;
  float3 f3;
  int ia[7];
};

// CHECK: OpName [[RECORD:%[^ ]*]] "RECORD"
// CHECK: OpMemberName [[RECORD]] 0 "h"
// CHECK: OpMemberName [[RECORD]] 1 "f"
// CHECK: OpMemberName [[RECORD]] 2 "d"
// CHECK: OpMemberName [[RECORD]] 3 "b"
// CHECK: OpMemberName [[RECORD]] 4 "i16"
// CHECK: OpMemberName [[RECORD]] 5 "u16"
// CHECK: OpMemberName [[RECORD]] 6 "i"
// CHECK: OpMemberName [[RECORD]] 7 "i64"
// CHECK: OpMemberName [[RECORD]] 8 "u64"
// CHECK: OpMemberName [[RECORD]] 9 "f3"
// CHECK: OpMemberName [[RECORD]] 10 "ia"

// CHECK-DAG: [[UINT:%[^ ]*]] = OpTypeInt 32 0
// CHECK-DAG: [[HALF:%[^ ]*]] = OpTypeFloat 16
// CHECK-DAG: [[INT:%[^ ]*]] = OpTypeInt 32 1
// CHECK-DAG: [[FLOAT:%[^ ]*]] = OpTypeFloat 32
// CHECK-DAG: [[DOUBLE:%[^ ]*]] = OpTypeFloat 64
// CHECK-DAG: [[SHORT:%[^ ]*]] = OpTypeInt 16 1
// CHECK-DAG: [[USHORT:%[^ ]*]] = OpTypeInt 16 0
// CHECK-DAG: [[LONG:%[^ ]*]] = OpTypeInt 64 1
// CHECK-DAG: [[ULONG:%[^ ]*]] = OpTypeInt 64 0
// CHECK-DAG: [[V3FLOAT:%[^ ]*]] = OpTypeVector [[FLOAT]] 3

// CHECK-DAG: [[U0:%[^ ]*]] = OpConstant [[UINT]] 0
// CHECK-DAG: [[U1:%[^ ]*]] = OpConstant [[UINT]] 1
// CHECK-DAG: [[HALF_0X1_8P_1:%[^ ]*]] = OpConstant [[HALF]] 0x1.8p+1
// CHECK-DAG: [[I0:%[^ ]*]] = OpConstant [[INT]] 0
// CHECK-DAG: [[FN5:%[^ ]*]] = OpConstant [[FLOAT]] -5
// CHECK-DAG: [[I1:%[^ ]*]] = OpConstant [[INT]] 1
// CHECK-DAG: [[D7:%[^ ]*]] = OpConstant [[DOUBLE]] 7
// CHECK-DAG: [[I2:%[^ ]*]] = OpConstant [[INT]] 2
// CHECK-DAG: [[I3:%[^ ]*]] = OpConstant [[INT]] 3
// CHECK-DAG: [[S11:%[^ ]*]] = OpConstant [[SHORT]] 11
// CHECK-DAG: [[I4:%[^ ]*]] = OpConstant [[INT]] 4
// CHECK-DAG: [[US13:%[^ ]*]] = OpConstant [[USHORT]] 13
// CHECK-DAG: [[I5:%[^ ]*]] = OpConstant [[INT]] 5
// CHECK-DAG: [[I17:%[^ ]*]] = OpConstant [[INT]] 17
// CHECK-DAG: [[I6:%[^ ]*]] = OpConstant [[INT]] 6
// CHECK-DAG: [[LN19:%[^ ]*]] = OpConstant [[LONG]] -19
// CHECK-DAG: [[I7:%[^ ]*]] = OpConstant [[INT]] 7
// CHECK-DAG: [[UL21:%[^ ]*]] = OpConstant [[ULONG]] 21
// CHECK-DAG: [[I8:%[^ ]*]] = OpConstant [[INT]] 8
// CHECK-DAG: [[F23:%[^ ]*]] = OpConstant [[FLOAT]] 23
// CHECK-DAG: [[I9:%[^ ]*]] = OpConstant [[INT]] 9
// CHECK-DAG: [[I29:%[^ ]*]] = OpConstant [[INT]] 29
// CHECK-DAG: [[I10:%[^ ]*]] = OpConstant [[INT]] 10
// CHECK-DAG: [[U7:%[^ ]*]] = OpConstant [[UINT]] 7

// CHECK-DAG: [[AI7:%[^ ]*]] = OpTypeArray [[INT]] [[U7]]
// CHECK-DAG: [[RECORD]] = OpTypeStruct [[HALF]] [[FLOAT]] [[DOUBLE]] [[UINT]] [[SHORT]] [[USHORT]] [[INT]] [[LONG]] [[ULONG]] [[V3FLOAT]] [[AI7]]
// CHECK-DAG: [[RAR:%[^ ]*]] = OpTypeNodePayloadArrayAMDX %RECORD
// CHECK-DAG: [[RARP:%[^ ]*]] = OpTypePointer NodePayloadAMDX [[RAR]]
// CHECK-DAG: [[U2:%[^ ]*]] = OpConstant [[UINT]] 2
// CHECK-DAG: [[HALFP:%[^ ]*]] = OpTypePointer Function [[HALF]]
// CHECK-DAG: [[FLOATP:%[^ ]*]] = OpTypePointer Function [[FLOAT]]
// CHECK-DAG: [[DOUBLEP:%[^ ]*]] = OpTypePointer Function [[DOUBLE]]
// CHECK-DAG: [[UINTP:%[^ ]*]] = OpTypePointer Function [[UINT]]
// CHECK-DAG: [[SHORTP:%[^ ]*]] = OpTypePointer Function [[SHORT]]
// CHECK-DAG: [[USHORTP:%[^ ]*]] = OpTypePointer Function [[USHORT]]
// CHECK-DAG: [[INTP:%[^ ]*]] = OpTypePointer Function [[INT]]
// CHECK-DAG: [[LONGP:%[^ ]*]] = OpTypePointer Function [[LONG]]
// CHECK-DAG: [[ULONGP:%[^ ]*]] = OpTypePointer Function [[ULONG]]

[Shader("node")]
[NumThreads(1024,1,1)]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(512,1,1)]
void node125(NodeOutput<RECORD> output)
{
  GroupNodeOutputRecords<RECORD> output01 = output.GetGroupNodeOutputRecords(1);
  // CHECK: OpAllocateNodePayloadsAMDX [[RARP]] [[U2]] [[U1]] [[U0]]

  output01.Get().h = 3.0;
  // CHECK: [[PTR:%[^ ]*]] = OpAccessChain [[HALFP]]
  // CHECK-SAME: [[I0]]
  // CHECK: OpStore [[PTR]] [[HALF_0X1_8P_1]]

  output01.Get().f = -5.0;
  // CHECK: [[PTR:%[^ ]*]] = OpAccessChain [[FLOATP]]
  // CHECK-SAME: [[I1]]
  // CHECK: OpStore [[PTR]] [[FN5]]

  output01.Get().d = 7.0;
  // CHECK: [[PTR:%[^ ]*]] = OpAccessChain [[DOUBLEP]]
  // CHECK-SAME: [[I2]]
  // CHECK: OpStore [[PTR]] [[D7]]

  output01.Get().b = true;
  // CHECK: [[PTR:%[^ ]*]] = OpAccessChain [[UINTP]]
  // CHECK-SAME: [[I3]]
  // CHECK: OpStore [[PTR]] [[U1]]

  output01.Get().i16 = 11;
  // CHECK: [[PTR:%[^ ]*]] = OpAccessChain [[SHORTP]]
  // CHECK-SAME: [[I4]]
  // CHECK: OpStore [[PTR]] [[S11]]

  output01.Get().u16 = 13;
  // CHECK: [[PTR:%[^ ]*]] = OpAccessChain [[USHORTP]]
  // CHECK-SAME: [[I5]]
  // CHECK: OpStore [[PTR]] [[US13]]

  output01.Get().i = 17;
  // CHECK: [[PTR:%[^ ]*]] = OpAccessChain [[INTP]]
  // CHECK-SAME: [[I6]]
  // CHECK: OpStore [[PTR]] [[I17]]

  output01.Get().i64 = -19;
  // CHECK: [[PTR:%[^ ]*]] = OpAccessChain [[LONGP]]
  // CHECK-SAME: [[I7]]
  // CHECK: OpStore [[PTR]] [[LN19]]

  output01.Get().u64 = 21;
  // CHECK: [[PTR:%[^ ]*]] = OpAccessChain [[ULONGP]]
  // CHECK-SAME: [[I8]]
  // CHECK: OpStore [[PTR]] [[UL21]]

  output01.Get().f3.y = 23;
  // CHECK: [[PTR:%[^ ]*]] = OpAccessChain [[FLOATP]]
  // CHECK-SAME: [[I9]]
  // CHECK-SAME: [[I1]]
  // CHECK: OpStore [[PTR]] [[F23]]

  output01.Get().ia[5] = 29;
  // CHECK: [[PTR:%[^ ]*]] = OpAccessChain [[INTP]]
  // CHECK-SAME: [[I10]]
  // CHECK-SAME: [[I5]]
  // CHECK: OpStore [[PTR]] [[I29]]
}
