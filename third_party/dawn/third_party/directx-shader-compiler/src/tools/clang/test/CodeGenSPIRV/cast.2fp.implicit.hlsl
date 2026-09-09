// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: [[v2float_1_0:%[0-9]+]] = OpConstantComposite %v2float %float_1 %float_0
// CHECK: [[v3float_0_4_n3:%[0-9]+]] = OpConstantComposite %v3float %float_0 %float_4 %float_n3
// CHECK: [[v3f1:%[0-9]+]] = OpConstantComposite %v3float %float_1 %float_1 %float_1
// CHECK: [[v3f0:%[0-9]+]] = OpConstantComposite %v3float %float_0 %float_0 %float_0

void main() {
// CHECK-LABEL: %bb_entry = OpLabel
    float f;
    bool from1;
    uint from2;
    int from3;

    float1 vf1;
    float2 vf2;
    float3 vf3;
    bool1 vfrom1;
    uint2 vfrom2;
    int3 vfrom3;

    // From constant (implicit)
// CHECK: OpStore %f %float_1
    f = true;
// CHECK-NEXT: OpStore %f %float_3
    f = 3u;

    // From constant expr
// CHECK-NEXT: OpStore %f %float_n1
    f = 5 - 6;

    // From variable (implicit)
// CHECK-NEXT: [[from1:%[0-9]+]] = OpLoad %bool %from1
// CHECK-NEXT: [[c1:%[0-9]+]] = OpSelect %float [[from1]] %float_1 %float_0
// CHECK-NEXT: OpStore %f [[c1]]
    f = from1;
// CHECK-NEXT: [[from2:%[0-9]+]] = OpLoad %uint %from2
// CHECK-NEXT: [[c2:%[0-9]+]] = OpConvertUToF %float [[from2]]
// CHECK-NEXT: OpStore %f [[c2]]
    f = from2;
// CHECK-NEXT: [[from3:%[0-9]+]] = OpLoad %int %from3
// CHECK-NEXT: [[c3:%[0-9]+]] = OpConvertSToF %float [[from3]]
// CHECK-NEXT: OpStore %f [[c3]]
    f = from3;

    // Vector cases

// CHECK: OpStore %vfc2 [[v2float_1_0]]
// CHECK: OpStore %vfc3 [[v3float_0_4_n3]]
    float2 vfc2 = {true, false};
    float3 vfc3 = {false, 4u, -3}; // Mixed

// CHECK-NEXT: [[vfrom1:%[0-9]+]] = OpLoad %bool %vfrom1
// CHECK-NEXT: [[vc1:%[0-9]+]] = OpSelect %float [[vfrom1]] %float_1 %float_0
// CHECK-NEXT: OpStore %vf1 [[vc1]]
    vf1 = vfrom1;
// CHECK-NEXT: [[vfrom2:%[0-9]+]] = OpLoad %v2uint %vfrom2
// CHECK-NEXT: [[vc2:%[0-9]+]] = OpConvertUToF %v2float [[vfrom2]]
// CHECK-NEXT: OpStore %vf2 [[vc2]]
    vf2 = vfrom2;
// CHECK-NEXT: [[vfrom3:%[0-9]+]] = OpLoad %v3int %vfrom3
// CHECK-NEXT: [[vc3:%[0-9]+]] = OpConvertSToF %v3float [[vfrom3]]
// CHECK-NEXT: OpStore %vf3 [[vc3]]
    vf3 = vfrom3;

// CHECK:                 [[a:%[0-9]+]] = OpLoad %bool %a
// CHECK-NEXT:        [[int_a:%[0-9]+]] = OpSelect %int [[a]] %int_1 %int_0
// CHECK-NEXT: [[zero_minus_a:%[0-9]+]] = OpISub %int %int_0 [[int_a]]
// CHECK-NEXT:              {{%[0-9]+}} = OpConvertSToF %float [[zero_minus_a]]
    bool a = false;
    float c = 0-a;

    int2x3   intMat;
    float2x3 floatMat;
    uint2x3  uintMat;
    bool2x3  boolMat;

// CHECK:        [[boolMat:%[0-9]+]] = OpLoad %_arr_v3bool_uint_2 %boolMat
// CHECK-NEXT:  [[boolMat0:%[0-9]+]] = OpCompositeExtract %v3bool [[boolMat]] 0
// CHECK-NEXT: [[floatMat0:%[0-9]+]] = OpSelect %v3float [[boolMat0]] [[v3f1]] [[v3f0]]
// CHECK-NEXT:  [[boolMat1:%[0-9]+]] = OpCompositeExtract %v3bool [[boolMat]] 1
// CHECK-NEXT: [[floatMat1:%[0-9]+]] = OpSelect %v3float [[boolMat1]] [[v3f1]] [[v3f0]]
// CHECK-NEXT:           {{%[0-9]+}} = OpCompositeConstruct %mat2v3float [[floatMat0]] [[floatMat1]]
    floatMat = boolMat;
// CHECK:        [[uintMat:%[0-9]+]] = OpLoad %_arr_v3uint_uint_2 %uintMat
// CHECK-NEXT:  [[uintMat0:%[0-9]+]] = OpCompositeExtract %v3uint [[uintMat]] 0
// CHECK-NEXT: [[floatMat0_0:%[0-9]+]] = OpConvertUToF %v3float [[uintMat0]]
// CHECK-NEXT:  [[uintMat1:%[0-9]+]] = OpCompositeExtract %v3uint [[uintMat]] 1
// CHECK-NEXT: [[floatMat1_0:%[0-9]+]] = OpConvertUToF %v3float [[uintMat1]]
// CHECK-NEXT:           {{%[0-9]+}} = OpCompositeConstruct %mat2v3float [[floatMat0_0]] [[floatMat1_0]]
    floatMat = uintMat;
// CHECK:         [[intMat:%[0-9]+]] = OpLoad %_arr_v3int_uint_2 %intMat
// CHECK-NEXT:   [[intMat0:%[0-9]+]] = OpCompositeExtract %v3int [[intMat]] 0
// CHECK-NEXT: [[floatMat0_1:%[0-9]+]] = OpConvertSToF %v3float [[intMat0]]
// CHECK-NEXT:   [[intMat1:%[0-9]+]] = OpCompositeExtract %v3int [[intMat]] 1
// CHECK-NEXT: [[floatMat1_1:%[0-9]+]] = OpConvertSToF %v3float [[intMat1]]
// CHECK-NEXT:           {{%[0-9]+}} = OpCompositeConstruct %mat2v3float [[floatMat0_1]] [[floatMat1_1]]
    floatMat = intMat;
}
