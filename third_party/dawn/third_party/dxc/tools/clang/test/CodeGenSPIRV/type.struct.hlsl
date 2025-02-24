// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK:      OpName %N "N"

// CHECK:      OpName %S "S"
// CHECK-NEXT: OpMemberName %S 0 "a"
// CHECK-NEXT: OpMemberName %S 1 "b"
// CHECK-NEXT: OpMemberName %S 2 "c"

// CHECK:      OpName %T "T"
// CHECK-NEXT: OpMemberName %T 0 "x"
// CHECK-NEXT: OpMemberName %T 1 "y"
// CHECK-NEXT: OpMemberName %T 2 "z"

// CHECK:      %X = OpTypeStruct %v4float %v3float
// CHECK:      %_struct_[[num:[0-9]+]] = OpTypeStruct %v2float
// CHECK:      %type_MyCBuffer = OpTypeStruct %X %_struct_[[num]]
cbuffer MyCBuffer {
  struct X {
    float4 a;
    float3 b;
  } x1;

  struct {
    float2 c;
  } y1;
};

// CHECK:      %type_MyTBuffer = OpTypeStruct %X %_struct_[[num]]
tbuffer MyTBuffer {
  X x2;

  struct {
    float2 c;
  } y2;
};

// CHECK:      %N = OpTypeStruct
struct N {};

// CHECK:      %S = OpTypeStruct %uint %v4float %mat2v3float
struct S {
  uint a;
  float4 b;
  float2x3 c;
};

// CHECK:      %T = OpTypeStruct %S %v3int %S
struct T {
  S x;
  int3 y;
  S z;
};

float4 main() : A {
  N n;
  S s;
  T t;

// CHECK: %R = OpTypeStruct %v2float

// CHECK: %r0 = OpVariable %_ptr_Function_R Function
  struct R {
    float2 rVal;
  } r0;

// CHECK: %r1 = OpVariable %_ptr_Function_R Function
  R r1;

  return x1.a + x2.a;
}
