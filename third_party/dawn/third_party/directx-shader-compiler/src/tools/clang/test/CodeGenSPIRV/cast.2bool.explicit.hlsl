// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: [[v2uint_1:%[0-9]+]] = OpConstantComposite %v2uint %uint_0 %uint_0
// CHECK: [[v3float_2:%[0-9]+]] = OpConstantComposite %v3float %float_0 %float_0 %float_0
// CHECK: [[v2bool_1_1:%[0-9]+]] = OpConstantComposite %v2bool %true %true

void main() {
// CHECK-LABEL: %bb_entry = OpLabel
    bool b;
    int from1;
    uint from2;
    float from3;

    bool1 vb1;
    bool2 vb2;
    bool3 vb3;
    int1 vfrom1;
    uint2 vfrom2;
    float3 vfrom3;

    // C style cast

    // From constant (explicit)
// CHECK: OpStore %b %true
    b = (bool)35;
// CHECK-NEXT: OpStore %b %false
    b = (bool)0.0;

    // From constant expr
// CHECK-NEXT: OpStore %b %true
    b = (bool)(3.5 - 3.4);

    // From variable (explicit)
// CHECK-NEXT: [[from1:%[0-9]+]] = OpLoad %int %from1
// CHECK-NEXT: [[c1:%[0-9]+]] = OpINotEqual %bool [[from1]] %int_0
// CHECK-NEXT: OpStore %b [[c1]]
    b = (bool)from1;
// CHECK-NEXT: [[from2:%[0-9]+]] = OpLoad %uint %from2
// CHECK-NEXT: [[c2:%[0-9]+]] = OpINotEqual %bool [[from2]] %uint_0
// CHECK-NEXT: OpStore %b [[c2]]
    b = (bool)from2;
// CHECK-NEXT: [[from3:%[0-9]+]] = OpLoad %float %from3
// CHECK-NEXT: [[c3:%[0-9]+]] = OpFOrdNotEqual %bool [[from3]] %float_0
// CHECK-NEXT: OpStore %b [[c3]]
    b = (bool)from3;

    // C++ function style cast

// CHECK-NEXT: OpStore %b %false
    b = bool(0);
// CHECK-NEXT: OpStore %b %true
    b = bool(3.5);

// CHECK-NEXT: OpStore %b %true
    b = bool(42 + 1);

// CHECK-NEXT: [[from4:%[0-9]+]] = OpLoad %int %from1
// CHECK-NEXT: [[c4:%[0-9]+]] = OpINotEqual %bool [[from4]] %int_0
// CHECK-NEXT: OpStore %b [[c4]]
    b = bool(from1);
// CHECK-NEXT: [[from5:%[0-9]+]] = OpLoad %uint %from2
// CHECK-NEXT: [[c5:%[0-9]+]] = OpINotEqual %bool [[from5]] %uint_0
// CHECK-NEXT: OpStore %b [[c5]]
    b = bool(from2);
// CHECK-NEXT: [[from6:%[0-9]+]] = OpLoad %float %from3
// CHECK-NEXT: [[c6:%[0-9]+]] = OpFOrdNotEqual %bool [[from6]] %float_0
// CHECK-NEXT: OpStore %b [[c6]]
    b = bool(from3);

    // Vector cases

// CHECK-NEXT: OpStore %vb1 %true
    vb1 = (bool1)42;
// CHECK-NEXT: [[vfrom1:%[0-9]+]] = OpLoad %int %vfrom1
// CHECK-NEXT: [[vc1:%[0-9]+]] = OpINotEqual %bool [[vfrom1]] %int_0
// CHECK-NEXT: OpStore %vb1 [[vc1]]
    vb1 = (bool1)vfrom1;
// CHECK-NEXT: [[vfrom2:%[0-9]+]] = OpLoad %v2uint %vfrom2
// CHECK-NEXT: [[vc2:%[0-9]+]] = OpINotEqual %v2bool [[vfrom2]] [[v2uint_1]]
// CHECK-NEXT: OpStore %vb2 [[vc2]]
    vb2 = (bool2)vfrom2;
// CHECK-NEXT: [[vfrom3:%[0-9]+]] = OpLoad %v3float %vfrom3
// CHECK-NEXT: [[vc3:%[0-9]+]] = OpFOrdNotEqual %v3bool [[vfrom3]] [[v3float_2]]
// CHECK-NEXT: OpStore %vb3 [[vc3]]
    vb3 = (bool3)vfrom3;

// CHECK-NEXT: OpStore %vb1 %true
    vb1 = bool1(3.5);
// CHECK-NEXT: OpStore %vb2 [[v2bool_1_1]]
    vb2 = bool2(1.1 + 1.2, 3.0);
// CHECK-NEXT: [[vfrom4:%[0-9]+]] = OpLoad %int %vfrom1
// CHECK-NEXT: [[vc4:%[0-9]+]] = OpINotEqual %bool [[vfrom4]] %int_0
// CHECK-NEXT: OpStore %vb1 [[vc4]]
    vb1 = bool1(vfrom1);
// CHECK-NEXT: [[vfrom5:%[0-9]+]] = OpLoad %v2uint %vfrom2
// CHECK-NEXT: [[vc5:%[0-9]+]] = OpINotEqual %v2bool [[vfrom5]] [[v2uint_1]]
// CHECK-NEXT: OpStore %vb2 [[vc5]]
    vb2 = bool2(vfrom2);
// CHECK-NEXT: [[vfrom6:%[0-9]+]] = OpLoad %v3float %vfrom3
// CHECK-NEXT: [[vc6:%[0-9]+]] = OpFOrdNotEqual %v3bool [[vfrom6]] [[v3float_2]]
// CHECK-NEXT: OpStore %vb3 [[vc6]]
    vb3 = bool3(vfrom3);
}
