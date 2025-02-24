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

// CHECK:      %N = OpTypeStruct
class N {};

// CHECK:      %S = OpTypeStruct %uint %v4float %mat2v3float
class S {
  uint a;
  float4 b;
  float2x3 c;
};

// CHECK:      %T = OpTypeStruct %S %v3int %S
class T {
  S x;
  int3 y;
  S z;
};

void main() {
  N n;
  S s;
  T t;

// CHECK: %R = OpTypeStruct %v2float
// CHECK: %r0 = OpVariable %_ptr_Function_R Function
  class R {
    float2 rVal;
  } r0;

// CHECK: %r1 = OpVariable %_ptr_Function_R Function
  R r1;
}
