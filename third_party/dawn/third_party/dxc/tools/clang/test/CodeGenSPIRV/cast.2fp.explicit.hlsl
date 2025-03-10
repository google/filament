// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: [[v2float_1_1:%[0-9]+]] = OpConstantComposite %v2float %float_1 %float_1
// CHECK: [[v2float_1:%[0-9]+]] = OpConstantComposite %v2float %float_0 %float_0
// CHECK: [[v3float_1_2_n3:%[0-9]+]] = OpConstantComposite %v3float %float_1 %float_2 %float_n3

void main() {
// CHECK-LABEL: %bb_entry = OpLabel
    float f;
    uint from1;
    bool from2;
    int from3;

    float1 vf1;
    float2 vf2;
    float3 vf3;
    uint1 vfrom1;
    bool2 vfrom2;
    int3 vfrom3;

    // C style cast

    // From constant (explicit)
// CHECK: OpStore %f %float_1
    f = (float)true;
// CHECK-NEXT: OpStore %f %float_4
    f = (float)4u;

    // From constant expr
// CHECK-NEXT: OpStore %f %float_n2
    f = (float)(3 - 5);

    // From variable (explicit)
// CHECK-NEXT: [[from1:%[0-9]+]] = OpLoad %uint %from1
// CHECK-NEXT: [[c1:%[0-9]+]] = OpConvertUToF %float [[from1]]
// CHECK-NEXT: OpStore %f [[c1]]
    f = (float)from1;
// CHECK-NEXT: [[from2:%[0-9]+]] = OpLoad %bool %from2
// CHECK-NEXT: [[c2:%[0-9]+]] = OpSelect %float [[from2]] %float_1 %float_0
// CHECK-NEXT: OpStore %f [[c2]]
    f = (float)from2;
// CHECK-NEXT: [[from3:%[0-9]+]] = OpLoad %int %from3
// CHECK-NEXT: [[c3:%[0-9]+]] = OpConvertSToF %float [[from3]]
// CHECK-NEXT: OpStore %f [[c3]]
    f = (float)from3;

    // C++ function style cast

// CHECK-NEXT: OpStore %f %float_0
    f = float(false);
// CHECK-NEXT: OpStore %f %float_2
    f = float(2u);

// CHECK-NEXT: OpStore %f %float_3
    f = float(-1 + 4);

// CHECK-NEXT: [[from4:%[0-9]+]] = OpLoad %uint %from1
// CHECK-NEXT: [[c4:%[0-9]+]] = OpConvertUToF %float [[from4]]
// CHECK-NEXT: OpStore %f [[c4]]
    f = float(from1);
// CHECK-NEXT: [[from5:%[0-9]+]] = OpLoad %bool %from2
// CHECK-NEXT: [[c5:%[0-9]+]] = OpSelect %float [[from5]] %float_1 %float_0
// CHECK-NEXT: OpStore %f [[c5]]
    f = float(from2);
// CHECK-NEXT: [[from6:%[0-9]+]] = OpLoad %int %from3
// CHECK-NEXT: [[c6:%[0-9]+]] = OpConvertSToF %float [[from6]]
// CHECK-NEXT: OpStore %f [[c6]]
    f = float(from3);

    // Vector cases

// CHECK-NEXT: OpStore %vf1 %float_7
    vf1 = (float1)7;
// CHECK-NEXT: [[vfrom1:%[0-9]+]] = OpLoad %uint %vfrom1
// CHECK-NEXT: [[vc1:%[0-9]+]] = OpConvertUToF %float [[vfrom1]]
// CHECK-NEXT: OpStore %vf1 [[vc1]]
    vf1 = (float1)vfrom1;
// CHECK-NEXT: [[vfrom2:%[0-9]+]] = OpLoad %v2bool %vfrom2
// CHECK-NEXT: [[vc2:%[0-9]+]] = OpSelect %v2float [[vfrom2]] [[v2float_1_1]] [[v2float_1]]
// CHECK-NEXT: OpStore %vf2 [[vc2]]
    vf2 = (float2)vfrom2;
// CHECK-NEXT: [[vfrom3:%[0-9]+]] = OpLoad %v3int %vfrom3
// CHECK-NEXT: [[vc3:%[0-9]+]] = OpConvertSToF %v3float [[vfrom3]]
// CHECK-NEXT: OpStore %vf3 [[vc3]]
    vf3 = (float3)vfrom3;

// CHECK-NEXT: OpStore %vf1 %float_3
    vf1 = float1(3);
// CHECK-NEXT: OpStore %vf3 [[v3float_1_2_n3]]
    vf3 = float3(true, 2u, -3);
// CHECK-NEXT: [[vfrom4:%[0-9]+]] = OpLoad %uint %vfrom1
// CHECK-NEXT: [[vc4:%[0-9]+]] = OpConvertUToF %float [[vfrom4]]
// CHECK-NEXT: OpStore %vf1 [[vc4]]
    vf1 = float1(vfrom1);
// CHECK-NEXT: [[vfrom5:%[0-9]+]] = OpLoad %v2bool %vfrom2
// CHECK-NEXT: [[vc5:%[0-9]+]] = OpSelect %v2float [[vfrom5]] [[v2float_1_1]] [[v2float_1]]
// CHECK-NEXT: OpStore %vf2 [[vc5]]
    vf2 = float2(vfrom2);
// CHECK-NEXT: [[vfrom6:%[0-9]+]] = OpLoad %v3int %vfrom3
// CHECK-NEXT: [[vc6:%[0-9]+]] = OpConvertSToF %v3float [[vfrom6]]
// CHECK-NEXT: OpStore %vf3 [[vc6]]
    vf3 = float3(vfrom3);
}

