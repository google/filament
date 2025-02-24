// RUN: %dxc -T ps_6_0 -E main -fspv-debug=vulkan -fcgl  %s -spirv | FileCheck %s

// CHECK:      [[file:%[0-9]+]] = OpString
// CHECK:      [[src:%[0-9]+]] = OpExtInst %void %2 DebugSource [[file]]

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
  // TODO(greg-lunarg): void inc() { a++; }
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
// CHECK:      DebugLine [[src]] %uint_51 %uint_51 %uint_7 %uint_19
// CHECK-NEXT: OpFunctionCall %int4_bool_float3_0 %test_struct
// CHECK:      OpVectorShuffle %v2float
      test_struct().c.zx
// CHECK:      DebugLine [[src]] %uint_46 %uint_46 %uint_12 %uint_12
// CHECK-NEXT: OpCompositeExtract %float {{%[0-9]+}} 0
// CHECK-NEXT: OpCompositeExtract %float {{%[0-9]+}} 1
// CHECK-NEXT: OpConvertFToS %int
// CHECK-NEXT: OpConvertFToS %int
// CHECK:      DebugLine [[src]] %uint_51 %uint_51 %uint_7 %uint_23
// CHECK-NEXT: OpCompositeExtract %float {{%[0-9]+}} 0
// CHECK-NEXT: OpCompositeExtract %float {{%[0-9]+}} 1
// CHECK:      DebugLine [[src]] %uint_46 %uint_46 %uint_12 %uint_12
// CHECK-NEXT: OpConvertFToS %int
// CHECK-NEXT: OpConvertFToS %int
// CHECK:      DebugLine [[src]] %uint_46 %uint_65 %uint_0 %uint_0
// CHECK:      OpCompositeConstruct %v4int
  };

// CHECK:                        OpFDiv %float {{%[0-9]+}} %float_2
// CHECK-NEXT:                   DebugLine [[src]] %uint_72 %uint_72 %uint_16 %uint_57
// CHECK-NEXT:  [[first:%[0-9]+]] = OpCompositeConstruct %v2float {{%[0-9]+}} {{%[0-9]+}}
// CHECK-NEXT: [[second:%[0-9]+]] = OpCompositeConstruct %v2float {{%[0-9]+}} {{%[0-9]+}}
// CHECK-NEXT:        {{%[0-9]+}} = OpCompositeConstruct %mat2v2float [[first]] [[second]]
  float2x2 b = float2x2(a.x, b._m00, 2 + a.y, b._m11 / 2);

// CHECK:                   DebugLine [[src]] %uint_77 %uint_77 %uint_12 %uint_14
// CHECK-NEXT: [[y:%[0-9]+]] = OpAccessChain %_ptr_Uniform_int4_bool_float3 %CONSTANTS %int_0
// CHECK-NEXT:   {{%[0-9]+}} = OpAccessChain %_ptr_Uniform_v4int [[y]] %int_0
  int4 c = y.a;

// CHECK:                   DebugLine [[src]] %uint_84 %uint_84 %uint_3 %uint_3
// CHECK-NEXT: [[z:%[0-9]+]] = OpLoad %type_2d_image %z
// CHECK-NEXT: [[z_0:%[0-9]+]] = OpImageRead %v4int [[z]] {{%[0-9]+}} None
// CHECK-NEXT: [[z_1:%[0-9]+]] = OpVectorShuffle %v3int [[z_0]] [[z_0]] 0 1 2
// CHECK:        {{%[0-9]+}} = OpCompositeInsert %v3int %int_16 [[z_1]] 0
  z[uint2(2, 3)].x = 16;

// CHECK:      DebugLine [[src]] %uint_90 %uint_90 %uint_3 %uint_4
// CHECK-NEXT: OpLoad %mat2v2float %b
// CHECK:      DebugLine [[src]] %uint_90 %uint_90 %uint_3 %uint_4
// CHECK-NEXT: OpFSub %v2float
  b--;

  int2x2 d;
// CHECK:      DebugLine [[src]] %uint_99 %uint_99 %uint_8 %uint_8
// CHECK-NEXT: OpLoad %mat2v2float %b
// CHECK-NEXT: DebugLine [[src]] %uint_99 %uint_99 %uint_3 %uint_12
// CHECK-NEXT: OpCompositeExtract %v2float
// CHECK:      OpCompositeConstruct %_arr_v2int_uint_2
// CHECK-NEXT: OpStore %d
  modf(b, d);

// CHECK-TODO:      DebugLine [[src]] %uint_103 %uint_103 %uint_3 %uint_11
// CHECK-NEXT-TODO: OpFunctionCall %void %S_inc %foo
// TODO(greg-lunarg):  foo.inc();

// CHECK-TODO:      DebugLine [[src]] %uint_107 %uint_107 %uint_3 %uint_14
// CHECK-NEXT-TODO: OpFunctionCall %void %S_inc %temp_var_S
// TODO(greg-lunarg):  getS().inc();

// CHECK:      DebugLine [[src]] %uint_113 %uint_113 %uint_19 %uint_19
// CHECK-NEXT: OpLoad %init %bar
// CHECK:      DebugLine [[src]] %uint_113 %uint_113 %uint_12 %uint_12
// CHECK-NEXT: OpConvertFToS %int
  int4 e = {1, 2, bar};

// CHECK:      DebugLine [[src]] %uint_119 %uint_119 %uint_7 %uint_25
// CHECK-NEXT: OpCompositeConstruct %v2float %float_1 %float_2
// CHECK-NEXT: DebugLine [[src]] %uint_119 %uint_119 %uint_22 %uint_22
// CHECK-NEXT: OpCompositeExtract %int
  b = float2x2(1, 2, bar);
// CHECK:      DebugLine [[src]] %uint_119 %uint_119 %uint_3 %uint_25
// CHECK-NEXT: OpStore %b

// TODO(jaebaek): Update InitListHandler to properly emit debug info.
  b = float2x2(c);
  c = int4(b);
}
