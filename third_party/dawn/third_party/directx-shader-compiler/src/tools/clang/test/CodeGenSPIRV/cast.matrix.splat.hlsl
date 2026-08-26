// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK:       [[v2f8_5:%[0-9]+]] = OpConstantComposite %v2float %float_8_5 %float_8_5
// CHECK:       [[v3f9_5:%[0-9]+]] = OpConstantComposite %v3float %float_9_5 %float_9_5 %float_9_5
// CHECK:      [[v2f10_5:%[0-9]+]] = OpConstantComposite %v2float %float_10_5 %float_10_5
// CHECK:    [[m3v2f10_5:%[0-9]+]] = OpConstantComposite %mat3v2float [[v2f10_5]] [[v2f10_5]] [[v2f10_5]]
// CHECK:        [[v2i10:%[0-9]+]] = OpConstantComposite %v2int %int_10 %int_10
// CHECK:   [[int3x2_i10:%[0-9]+]] = OpConstantComposite %_arr_v2int_uint_3 [[v2i10]] [[v2i10]] [[v2i10]]
// CHECK:       [[v2true:%[0-9]+]] = OpConstantComposite %v2bool %true %true
// CHECK: [[bool3x2_true:%[0-9]+]] = OpConstantComposite %_arr_v2bool_uint_3 [[v2true]] [[v2true]] [[v2true]]

void main() {
// CHECK-LABEL: %bb_entry = OpLabel

    // TODO: Optimally the following literals can be attached to variable
    // definitions instead of OpStore. Constant evaluation in the front
    // end doesn't really support it for now.

// CHECK:      OpStore %a %float_7_5
    float1x1 a = 7.5;
// CHECK-NEXT: OpStore %b [[v2f8_5]]
    float1x2 b = 8.5;
// CHECK-NEXT: OpStore %c [[v3f9_5]]
    float3x1 c = 9.5;
// CHECK-NEXT: OpStore %d [[m3v2f10_5]]
    float3x2 d = 10.5;
// CHECK-NEXT: OpStore %e [[int3x2_i10]]
      int3x2 e = 10;
// CHECK-NEXT: OpStore %f [[bool3x2_true]]
     bool3x2 f = true;

    float val;
// CHECK-NEXT: [[val0:%[0-9]+]] = OpLoad %float %val
// CHECK-NEXT: OpStore %h [[val0]]
    float1x1 h = val;
// CHECK-NEXT: [[val1:%[0-9]+]] = OpLoad %float %val
// CHECK-NEXT: [[cc0:%[0-9]+]] = OpCompositeConstruct %v3float [[val1]] [[val1]] [[val1]]
// CHECK-NEXT: OpStore %i [[cc0]]
    float1x3 i = val;
    float2x1 j;
    float2x3 k;

// CHECK-NEXT: [[val2:%[0-9]+]] = OpLoad %float %val
// CHECK-NEXT: [[cc1:%[0-9]+]] = OpCompositeConstruct %v2float [[val2]] [[val2]]
// CHECK-NEXT: OpStore %j [[cc1]]
    j = val;
// CHECK-NEXT: [[val3:%[0-9]+]] = OpLoad %float %val
// CHECK-NEXT: [[cc2:%[0-9]+]] = OpCompositeConstruct %v3float [[val3]] [[val3]] [[val3]]
// CHECK-NEXT: [[cc3:%[0-9]+]] = OpCompositeConstruct %mat2v3float [[cc2]] [[cc2]]
// CHECK-NEXT: OpStore %k [[cc3]]
    k = val;

    int intVal;
// CHECK:      [[intVal:%[0-9]+]] = OpLoad %int %intVal
// CHECK-NEXT:    [[cc4:%[0-9]+]] = OpCompositeConstruct %v3int [[intVal]] [[intVal]] [[intVal]]
// CHECK-NEXT: OpStore %m [[cc4]]
    int1x3 m = intVal;
    int2x1 n;
    int2x3 o;
// CHECK:      [[intVal_0:%[0-9]+]] = OpLoad %int %intVal
// CHECK-NEXT:    [[cc5:%[0-9]+]] = OpCompositeConstruct %v2int [[intVal_0]] [[intVal_0]]
// CHECK-NEXT: OpStore %n [[cc5]]
    n = intVal;
// CHECK:        [[intVal_1:%[0-9]+]] = OpLoad %int %intVal
// CHECK-NEXT: [[v3intVal:%[0-9]+]] = OpCompositeConstruct %v3int [[intVal_1]] [[intVal_1]] [[intVal_1]]
// CHECK-NEXT:      [[cc6:%[0-9]+]] = OpCompositeConstruct %_arr_v3int_uint_2 [[v3intVal]] [[v3intVal]]
// CHECK-NEXT: OpStore %o [[cc6]]
    o = intVal;

    bool boolVal;
// CHECK:      [[boolVal:%[0-9]+]] = OpLoad %bool %boolVal
// CHECK-NEXT:     [[cc7:%[0-9]+]] = OpCompositeConstruct %v3bool [[boolVal]] [[boolVal]] [[boolVal]]
// CHECK-NEXT: OpStore %p [[cc7]]
    bool1x3 p = boolVal;
    bool2x1 q;
    bool2x3 r;
// CHECK:      [[boolVal_0:%[0-9]+]] = OpLoad %bool %boolVal
// CHECK-NEXT:     [[cc8:%[0-9]+]] = OpCompositeConstruct %v2bool [[boolVal_0]] [[boolVal_0]]
// CHECK-NEXT: OpStore %q [[cc8]]
    q = boolVal;
// CHECK:        [[boolVal_1:%[0-9]+]] = OpLoad %bool %boolVal
// CHECK-NEXT: [[v3boolVal:%[0-9]+]] = OpCompositeConstruct %v3bool [[boolVal_1]] [[boolVal_1]] [[boolVal_1]]
// CHECK-NEXT:       [[cc9:%[0-9]+]] = OpCompositeConstruct %_arr_v3bool_uint_2 [[v3boolVal]] [[v3boolVal]]
// CHECK-NEXT: OpStore %r [[cc9]]
    r = boolVal;
}
