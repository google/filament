// RUN: %dxc -T ps_6_0 -E main -Zi -fcgl  %s -spirv | FileCheck %s

// CHECK:      [[file:%[0-9]+]] = OpString
// CHECK-SAME: spirv.debug.opline.function.hlsl

void foo(in float4 a, out float3 b);

// CHECK:                  OpLine [[file]] 28 1
// CHECK-NEXT: %main = OpFunction %void None

void bar(int a, in float b, inout bool2 c, const float3 d, out uint4 e) {
}

struct R {
  int a;
  void incr();
  void decr() { --a; }
};

RWStructuredBuffer<R> rwsb;

void decr(inout R a, in R b, out R c, R d, const R e);

static R r[5];

R getR(uint i);

void main() {
  float4 v4f;
  float3 v3f;

  foo(v4f, v3f);
  r[0].incr();
  decr(r[0], getR(1), r[2], getR(3), getR(4));

  rwsb[0].incr();
}

// CHECK:             OpLine [[file]] 45 1
// CHECK-NEXT: %foo = OpFunction %void None
// CHECK-NEXT:        OpLine [[file]] 45 20
// CHECK-NEXT:   %a = OpFunctionParameter %_ptr_Function_v4float
// CHECK-NEXT:        OpLine [[file]] 45 34
// CHECK-NEXT:   %b = OpFunctionParameter %_ptr_Function_v3float
void foo(in float4 a, out float3 b) {
  a = b.xxzz;
  b = a.yzw;
  bar(a.x, b.y, a.yz, b, a);
}

// CHECK:                     OpLine [[file]] 54 1
// CHECK-NEXT:      %R_incr = OpFunction %void None
// CHECK-NEXT:  %param_this = OpFunctionParameter %_ptr_Function_R
void R::incr() { ++a; }

// CHECK:                     OpLine [[file]] 60 1
// CHECK-NEXT:        %getR = OpFunction %R None
// CHECK-NEXT:                OpLine [[file]] 60 13
// CHECK-NEXT:           %i = OpFunctionParameter %_ptr_Function_uint
R getR(uint i) { return r[i]; }

// CHECK:                     OpLine [[file]] 68 1
// CHECK-NEXT:        %decr = OpFunction %void None
// CHECK-NEXT:                OpLine [[file]] 68 19
// CHECK-NEXT:         %a_0 = OpFunctionParameter %_ptr_Function_R
// CHECK-NEXT:                OpLine [[file]] 68 27
// CHECK-NEXT:         %b_0 = OpFunctionParameter %_ptr_Function_R
void decr(inout R a, in R b, out R c, R d, const R e) { a.a--; }

// CHECK:             OpLine [[file]] 11 1
// CHECK-NEXT: %bar = OpFunction %void None
// CHECK-NEXT:        OpLine [[file]] 11 14
// CHECK-NEXT: %a_1 = OpFunctionParameter %_ptr_Function_int
// CHECK-NEXT:        OpLine [[file]] 11 26
// CHECK-NEXT: %b_1 = OpFunctionParameter %_ptr_Function_float
// CHECK-NEXT:        OpLine [[file]] 11 41
// CHECK-NEXT: %c_0 = OpFunctionParameter %_ptr_Function_v2bool
// CHECK-NEXT:        OpLine [[file]] 11 57
// CHECK-NEXT: %d_0 = OpFunctionParameter %_ptr_Function_v3float
// CHECK-NEXT:        OpLine [[file]] 11 70
// CHECK-NEXT: %e_0 = OpFunctionParameter %_ptr_Function_v4uint
