// RUN: %dxc -T ps_6_0 -E main -Zi -fcgl  %s -spirv | FileCheck %s

// CHECK:      [[file:%[0-9]+]] = OpString
// CHECK-SAME: spirv.debug.opline.composite.hlsl

struct int4_bool_float3 {
  int4 a;
  bool b;
  float3 c;
};

int4_bool_float3 test_struct() {
  int4_bool_float3 x;
  return x;
}

cbuffer CONSTANTS {
  int4_bool_float3 y;
};

RWTexture2D<int3> z;

struct S {
  int a;
  void inc() { a++; }
};

S getS() {
  S a;
  return a;
}

struct init {
  int first;
  float second;
};

// Note that preprocessor prepends a "#line 1 ..." line to the whole file,
// the compliation sees line numbers incremented by 1.

void main() {
  S foo;

  init bar;

  int4 a = {
      float2(1, 0),
// CHECK: OpLine [[file]] 50 7
// CHECK: OpFunctionCall %int4_bool_float3_0 %test_struct
      test_struct().c.zx
// CHECK:      OpLine [[file]] 46 12
// CHECK:      OpCompositeExtract %float {{%[0-9]+}} 0
// CHECK-NEXT: OpCompositeExtract %float {{%[0-9]+}} 1
// CHECK-NEXT: OpConvertFToS %int
// CHECK-NEXT: OpConvertFToS %int
// CHECK-NEXT: OpCompositeExtract %float {{%[0-9]+}} 0
// CHECK-NEXT: OpCompositeExtract %float {{%[0-9]+}} 1
// CHECK-NEXT: OpConvertFToS %int
// CHECK-NEXT: OpConvertFToS %int
// CHECK-NEXT: OpCompositeConstruct %v4int
  };

// CHECK:                        OpFDiv %float {{%[0-9]+}} %float_2
// CHECK-NEXT:                   OpLine [[file]] 68 24
// CHECK-NEXT:  [[first:%[0-9]+]] = OpCompositeConstruct %v2float {{%[0-9]+}} {{%[0-9]+}}
// CHECK-NEXT: [[second:%[0-9]+]] = OpCompositeConstruct %v2float {{%[0-9]+}} {{%[0-9]+}}
// CHECK-NEXT:        {{%[0-9]+}} = OpCompositeConstruct %mat2v2float [[first]] [[second]]
  float2x2 b = float2x2(a.x, b._m00, 2 + a.y, b._m11 / 2);

// CHECK:                   OpLine [[file]] 73 12
// CHECK-NEXT: [[y:%[0-9]+]] = OpAccessChain %_ptr_Uniform_int4_bool_float3 %CONSTANTS %int_0
// CHECK-NEXT:   {{%[0-9]+}} = OpAccessChain %_ptr_Uniform_v4int [[y]] %int_0
  int4 c = y.a;

// CHECK:                   OpLine [[file]] 80 3
// CHECK-NEXT: [[z:%[0-9]+]] = OpLoad %type_2d_image %z
// CHECK-NEXT: [[z_0:%[0-9]+]] = OpImageRead %v4int [[z]] {{%[0-9]+}} None
// CHECK-NEXT: [[z_1:%[0-9]+]] = OpVectorShuffle %v3int [[z_0]] [[z_0]] 0 1 2
// CHECK-NEXT:   {{%[0-9]+}} = OpCompositeInsert %v3int %int_16 [[z_1]] 0
  z[uint2(2, 3)].x = 16;

// CHECK:      OpLine [[file]] 86 3
// CHECK-NEXT: OpLoad %mat2v2float %b
// CHECK:      OpLine [[file]] 86 4
// CHECK-NEXT: OpFSub %v2float
  b--;

  int2x2 d;
// CHECK:      OpLine [[file]] 95 8
// CHECK-NEXT: OpLoad %mat2v2float %b
// CHECK-NEXT: OpLine [[file]] 95 3
// CHECK-NEXT: OpCompositeExtract %v2float
// CHECK:      OpLine [[file]] 95 11
// CHECK:      OpStore %d
  modf(b, d);

// CHECK:      OpLine [[file]] 99 7
// CHECK-NEXT: OpFunctionCall %void %S_inc %foo
  foo.inc();

// CHECK:      OpLine [[file]] 103 10
// CHECK-NEXT: OpFunctionCall %void %S_inc %temp_var_S
  getS().inc();

// CHECK:      OpLine [[file]] 109 19
// CHECK-NEXT: OpLoad %init %bar
// CHECK:      OpLine [[file]] 109 12
// CHECK-NEXT: OpConvertFToS %int
  int4 e = {1, 2, bar};

// CHECK:      OpLine [[file]] 115 15
// CHECK-NEXT: OpCompositeConstruct %v2float %float_1 %float_2
// CHECK-NEXT: OpLine [[file]] 115 22
// CHECK-NEXT: OpCompositeExtract %int
  b = float2x2(1, 2, bar);
// CHECK:      OpLine [[file]] 115 3
// CHECK-NEXT: OpStore %b

// TODO(jaebaek): Update InitListHandler to properly emit debug info.
  b = float2x2(c);
  c = int4(b);
}
