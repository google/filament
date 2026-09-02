// RUN: %dxc -T cs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// A and B are groupshared, and should not be placed in the $Globals cbuffer.
// myGlobalInteger is a global variable and is placed in $Globals cbuffer.

// CHECK:    OpMemberName %type__Globals 0 "myGlobalInteger"
// CHECK-NOT:OpMemberName %type__Globals 1

// CHECK: OpMemberName %type_myCbuffer 0 "myA"
// CHECK: OpMemberName %type_myCbuffer 1 "myB"
// CHECK: OpMemberName %type_myCbuffer 2 "myC"

// CHECK:      OpMemberDecorate %type__Globals 0 Offset
// CHECK-NOT:  OpMemberDecorate %type__Globals 1 Offset
// CHECK-NOT:  OpMemberDecorate %type__Globals 2 Offset

// CHECK:      OpMemberDecorate %type_myCbuffer 0 Offset
// CHECK-NEXT: OpMemberDecorate %type_myCbuffer 1 Offset
// CHECK-NEXT: OpMemberDecorate %type_myCbuffer 2 Offset

// CHECK: %type__Globals = OpTypeStruct %int
// CHECK: %type_myCbuffer = OpTypeStruct %int %int %int

// CHECK: %A = OpVariable %_ptr_Workgroup_int Workgroup
groupshared int A;

// CHECK: %B = OpVariable %_ptr_Workgroup__arr_int_uint_10 Workgroup
groupshared int B[10];

// CHECK: %_Globals = OpVariable %_ptr_Uniform_type__Globals Uniform
int myGlobalInteger;

// CHECK: %myCbuffer = OpVariable %_ptr_Uniform_type_myCbuffer Uniform
cbuffer myCbuffer {
  int myA;
  int myB;
  groupshared int myC;
}

RWTexture2D<float> g_OutputSingleChannel : register(u0);

[numthreads(8, 8, 1)]
void main(uint2 DTid : SV_DispatchThreadID) {
  g_OutputSingleChannel[DTid + myA + myB + myC + B[0] + A + myGlobalInteger] = 0.0;
}
