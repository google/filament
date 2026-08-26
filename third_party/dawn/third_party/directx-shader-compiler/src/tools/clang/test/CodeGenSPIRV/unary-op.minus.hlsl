// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

void main() {
// CHECK-LABEL: %bb_entry = OpLabel
    int a, b;
// CHECK:      [[a0:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: [[b0:%[0-9]+]] = OpSNegate %int [[a0]]
// CHECK-NEXT: OpStore %b [[b0]]
    b = -a;

    uint c, d;
// CHECK-NEXT: [[c0:%[0-9]+]] = OpLoad %uint %c
// CHECK-NEXT: [[d0:%[0-9]+]] = OpSNegate %uint [[c0]]
// CHECK-NEXT: OpStore %d [[d0]]
    d = -c;

    float i, j;
// CHECK-NEXT: [[i0:%[0-9]+]] = OpLoad %float %i
// CHECK-NEXT: [[j0:%[0-9]+]] = OpFNegate %float [[i0]]
// CHECK-NEXT: OpStore %j [[j0]]
    j = -i;

    float1 m, n;
// CHECK-NEXT: [[m0:%[0-9]+]] = OpLoad %float %m
// CHECK-NEXT: [[n0:%[0-9]+]] = OpFNegate %float [[m0]]
// CHECK-NEXT: OpStore %n [[n0]]
    n = -m;

    int3 x, y;
// CHECK-NEXT: [[x0:%[0-9]+]] = OpLoad %v3int %x
// CHECK-NEXT: [[y0:%[0-9]+]] = OpSNegate %v3int [[x0]]
// CHECK-NEXT: OpStore %y [[y0]]
    y = -x;

// CHECK-NEXT:       [[s:%[0-9]+]] = OpLoad %_arr_v4int_uint_4 %s
// CHECK-NEXT:    [[row0:%[0-9]+]] = OpCompositeExtract %v4int [[s]] 0
// CHECK-NEXT: [[result0:%[0-9]+]] = OpSNegate %v4int [[row0]]
// CHECK-NEXT:    [[row1:%[0-9]+]] = OpCompositeExtract %v4int [[s]] 1
// CHECK-NEXT: [[result1:%[0-9]+]] = OpSNegate %v4int [[row1]]
// CHECK-NEXT:    [[row2:%[0-9]+]] = OpCompositeExtract %v4int [[s]] 2
// CHECK-NEXT: [[result2:%[0-9]+]] = OpSNegate %v4int [[row2]]
// CHECK-NEXT:    [[row3:%[0-9]+]] = OpCompositeExtract %v4int [[s]] 3
// CHECK-NEXT: [[result3:%[0-9]+]] = OpSNegate %v4int [[row3]]
// CHECK-NEXT:  [[result:%[0-9]+]] = OpCompositeConstruct %_arr_v4int_uint_4 [[result0]] [[result1]] [[result2]] [[result3]]
// CHECK-NEXT:                    OpStore %r [[result]]
    int4x4 r, s;
    r = -s;

// CHECK-NEXT:       [[u:%[0-9]+]] = OpLoad %mat4v4float %u
// CHECK-NEXT:    [[row0_0:%[0-9]+]] = OpCompositeExtract %v4float [[u]] 0
// CHECK-NEXT: [[result0_0:%[0-9]+]] = OpFNegate %v4float [[row0_0]]
// CHECK-NEXT:    [[row1_0:%[0-9]+]] = OpCompositeExtract %v4float [[u]] 1
// CHECK-NEXT: [[result1_0:%[0-9]+]] = OpFNegate %v4float [[row1_0]]
// CHECK-NEXT:    [[row2_0:%[0-9]+]] = OpCompositeExtract %v4float [[u]] 2
// CHECK-NEXT: [[result2_0:%[0-9]+]] = OpFNegate %v4float [[row2_0]]
// CHECK-NEXT:    [[row3_0:%[0-9]+]] = OpCompositeExtract %v4float [[u]] 3
// CHECK-NEXT: [[result3_0:%[0-9]+]] = OpFNegate %v4float [[row3_0]]
// CHECK-NEXT:  [[result_0:%[0-9]+]] = OpCompositeConstruct %mat4v4float [[result0_0]] [[result1_0]] [[result2_0]] [[result3_0]]
// CHECK-NEXT:                    OpStore %t [[result_0]]
    float4x4 t, u;
    t = -u;
}
