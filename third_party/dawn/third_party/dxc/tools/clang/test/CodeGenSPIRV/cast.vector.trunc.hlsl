// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: [[v3f32c0:%[0-9]+]] = OpConstantComposite %v3float %float_1 %float_2 %float_3
// CHECK: [[v3f32c1:%[0-9]+]] = OpConstantComposite %v3float %float_5 %float_6 %float_7
// CHECK:  [[v2f32c:%[0-9]+]] = OpConstantComposite %v2float %float_8 %float_9

void main() {
// CHECK-LABEL: %bb_entry = OpLabel

    // From constant
// CHECK: OpStore %vf3 [[v3f32c0]]
    float3 vf3 = float4(1, 2, 3, 4);
// CHECK-NEXT: [[c5:%[0-9]+]] = OpCompositeExtract %float [[v3f32c1]] 0
// CHECK-NEXT: OpStore %vf1 [[c5]]
    float1 vf1;
    vf1 = float3(5, 6, 7);
// CHECK-NEXT: [[c8:%[0-9]+]] = OpCompositeExtract %float [[v2f32c]] 0
// CHECK-NEXT: OpStore %sfa [[c8]]
    float sfa = float2(8, 9);
// CHECK-NEXT: OpStore %sfb %float_10
    float sfb;
    sfb = float1(10);

    // From variable
    int4 vi4;
// CHECK-NEXT: [[vi4:%[0-9]+]] = OpLoad %v4int %vi4
// CHECK-NEXT: [[e1:%[0-9]+]] = OpCompositeExtract %int [[vi4]] 0
// CHECK-NEXT: [[e2:%[0-9]+]] = OpCompositeExtract %int [[vi4]] 1
// CHECK-NEXT: [[e3:%[0-9]+]] = OpCompositeExtract %int [[vi4]] 2
// CHECK-NEXT: [[vi3:%[0-9]+]] = OpCompositeConstruct %v3int [[e1]] [[e2]] [[e3]]
// CHECK-NEXT: OpStore %vi3 [[vi3]]
    int3 vi3;
    vi3 = vi4;
// CHECK-NEXT: [[vi3_1:%[0-9]+]] = OpLoad %v3int %vi3
// CHECK-NEXT: [[e4:%[0-9]+]] = OpCompositeExtract %int [[vi3_1]] 0
// CHECK-NEXT: OpStore %vi1 [[e4]]
    int1 vi1 = vi3;
// CHECK-NEXT: [[vi3_2:%[0-9]+]] = OpLoad %v3int %vi3
// CHECK-NEXT: [[e5:%[0-9]+]] = OpCompositeExtract %int [[vi3_2]] 0
// CHECK-NEXT: OpStore %sia [[e5]]
    int sia;
    sia = vi3;
// CHECK-NEXT: [[vi1:%[0-9]+]] = OpLoad %int %vi1
// CHECK-NEXT: OpStore %sib [[vi1]]
    int sib = vi1;

    // Used in expression

// CHECK-NEXT: [[sia:%[0-9]+]] = OpLoad %int %sia
// CHECK-NEXT: [[cc0:%[0-9]+]] = OpCompositeConstruct %v3int [[sia]] [[sia]] [[sia]]
// CHECK-NEXT: [[vi3_3:%[0-9]+]] = OpLoad %v3int %vi3
// CHECK-NEXT: [[add:%[0-9]+]] = OpIAdd %v3int [[cc0]] [[vi3_3]]
// CHECK-NEXT: [[e6:%[0-9]+]] = OpCompositeExtract %int [[add]] 0
// CHECK-NEXT: OpStore %sib [[e6]]
    sib = sia + vi3;
}
