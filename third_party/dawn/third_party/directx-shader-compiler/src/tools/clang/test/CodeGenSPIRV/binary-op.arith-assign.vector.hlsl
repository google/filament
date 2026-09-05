// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: [[v3i4:%[0-9]+]] = OpConstantComposite %v3int %int_4 %int_4 %int_4

void main() {
// CHECK-LABEL: %bb_entry = OpLabel

    int1 a, b, c, d;
    int2 i, j;
    uint3 o, p;
    float4 x, y;

// CHECK:      [[a0:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: [[b0:%[0-9]+]] = OpLoad %int %b
// CHECK-NEXT: [[add0:%[0-9]+]] = OpIAdd %int [[b0]] [[a0]]
// CHECK-NEXT: OpStore %b [[add0]]
    b += a;
// CHECK-NEXT: [[i0:%[0-9]+]] = OpLoad %v2int %i
// CHECK-NEXT: [[j0:%[0-9]+]] = OpLoad %v2int %j
// CHECK-NEXT: [[add1:%[0-9]+]] = OpIAdd %v2int [[j0]] [[i0]]
// CHECK-NEXT: OpStore %j [[add1]]
    j += i;
// CHECK-NEXT: [[o0:%[0-9]+]] = OpLoad %v3uint %o
// CHECK-NEXT: [[p0:%[0-9]+]] = OpLoad %v3uint %p
// CHECK-NEXT: [[add2:%[0-9]+]] = OpIAdd %v3uint [[p0]] [[o0]]
// CHECK-NEXT: OpStore %p [[add2]]
    p += o;
// CHECK-NEXT: [[x0:%[0-9]+]] = OpLoad %v4float %x
// CHECK-NEXT: [[y0:%[0-9]+]] = OpLoad %v4float %y
// CHECK-NEXT: [[add3:%[0-9]+]] = OpFAdd %v4float [[y0]] [[x0]]
// CHECK-NEXT: OpStore %y [[add3]]
    y += x;

// CHECK-NEXT: [[a1:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: [[b1:%[0-9]+]] = OpLoad %int %b
// CHECK-NEXT: [[sub0:%[0-9]+]] = OpISub %int [[b1]] [[a1]]
// CHECK-NEXT: OpStore %b [[sub0]]
    b -= a;
// CHECK-NEXT: [[i1:%[0-9]+]] = OpLoad %v2int %i
// CHECK-NEXT: [[j1:%[0-9]+]] = OpLoad %v2int %j
// CHECK-NEXT: [[sub1:%[0-9]+]] = OpISub %v2int [[j1]] [[i1]]
// CHECK-NEXT: OpStore %j [[sub1]]
    j -= i;
// CHECK-NEXT: [[o1:%[0-9]+]] = OpLoad %v3uint %o
// CHECK-NEXT: [[p1:%[0-9]+]] = OpLoad %v3uint %p
// CHECK-NEXT: [[sub2:%[0-9]+]] = OpISub %v3uint [[p1]] [[o1]]
// CHECK-NEXT: OpStore %p [[sub2]]
    p -= o;
// CHECK-NEXT: [[x1:%[0-9]+]] = OpLoad %v4float %x
// CHECK-NEXT: [[y1:%[0-9]+]] = OpLoad %v4float %y
// CHECK-NEXT: [[sub3:%[0-9]+]] = OpFSub %v4float [[y1]] [[x1]]
// CHECK-NEXT: OpStore %y [[sub3]]
    y -= x;

// CHECK-NEXT: [[a2:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: [[b2:%[0-9]+]] = OpLoad %int %b
// CHECK-NEXT: [[mul0:%[0-9]+]] = OpIMul %int [[b2]] [[a2]]
// CHECK-NEXT: OpStore %b [[mul0]]
    b *= a;
// CHECK-NEXT: [[i2:%[0-9]+]] = OpLoad %v2int %i
// CHECK-NEXT: [[j2:%[0-9]+]] = OpLoad %v2int %j
// CHECK-NEXT: [[mul1:%[0-9]+]] = OpIMul %v2int [[j2]] [[i2]]
// CHECK-NEXT: OpStore %j [[mul1]]
    j *= i;
// CHECK-NEXT: [[o2:%[0-9]+]] = OpLoad %v3uint %o
// CHECK-NEXT: [[p2:%[0-9]+]] = OpLoad %v3uint %p
// CHECK-NEXT: [[mul2:%[0-9]+]] = OpIMul %v3uint [[p2]] [[o2]]
// CHECK-NEXT: OpStore %p [[mul2]]
    p *= o;
// CHECK-NEXT: [[x2:%[0-9]+]] = OpLoad %v4float %x
// CHECK-NEXT: [[y2:%[0-9]+]] = OpLoad %v4float %y
// CHECK-NEXT: [[mul3:%[0-9]+]] = OpFMul %v4float [[y2]] [[x2]]
// CHECK-NEXT: OpStore %y [[mul3]]
    y *= x;

// CHECK-NEXT: [[a4:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: [[b4:%[0-9]+]] = OpLoad %int %b
// CHECK-NEXT: [[div0:%[0-9]+]] = OpSDiv %int [[b4]] [[a4]]
// CHECK-NEXT: OpStore %b [[div0]]
    b /= a;
// CHECK-NEXT: [[i4:%[0-9]+]] = OpLoad %v2int %i
// CHECK-NEXT: [[j4:%[0-9]+]] = OpLoad %v2int %j
// CHECK-NEXT: [[div1:%[0-9]+]] = OpSDiv %v2int [[j4]] [[i4]]
// CHECK-NEXT: OpStore %j [[div1]]
    j /= i;
// CHECK-NEXT: [[o4:%[0-9]+]] = OpLoad %v3uint %o
// CHECK-NEXT: [[p4:%[0-9]+]] = OpLoad %v3uint %p
// CHECK-NEXT: [[div2:%[0-9]+]] = OpUDiv %v3uint [[p4]] [[o4]]
// CHECK-NEXT: OpStore %p [[div2]]
    p /= o;
// CHECK-NEXT: [[x4:%[0-9]+]] = OpLoad %v4float %x
// CHECK-NEXT: [[y4:%[0-9]+]] = OpLoad %v4float %y
// CHECK-NEXT: [[div3:%[0-9]+]] = OpFDiv %v4float [[y4]] [[x4]]
// CHECK-NEXT: OpStore %y [[div3]]
    y /= x;

// CHECK-NEXT: [[a5:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: [[b5:%[0-9]+]] = OpLoad %int %b
// CHECK-NEXT: [[mod0:%[0-9]+]] = OpSRem %int [[b5]] [[a5]]
// CHECK-NEXT: OpStore %b [[mod0]]
    b %= a;
// CHECK-NEXT: [[i5:%[0-9]+]] = OpLoad %v2int %i
// CHECK-NEXT: [[j5:%[0-9]+]] = OpLoad %v2int %j
// CHECK-NEXT: [[mod1:%[0-9]+]] = OpSRem %v2int [[j5]] [[i5]]
// CHECK-NEXT: OpStore %j [[mod1]]
    j %= i;
// CHECK-NEXT: [[o5:%[0-9]+]] = OpLoad %v3uint %o
// CHECK-NEXT: [[p5:%[0-9]+]] = OpLoad %v3uint %p
// CHECK-NEXT: [[mod2:%[0-9]+]] = OpUMod %v3uint [[p5]] [[o5]]
// CHECK-NEXT: OpStore %p [[mod2]]
    p %= o;
// CHECK-NEXT: [[x5:%[0-9]+]] = OpLoad %v4float %x
// CHECK-NEXT: [[y5:%[0-9]+]] = OpLoad %v4float %y
// CHECK-NEXT: [[mod3:%[0-9]+]] = OpFRem %v4float [[y5]] [[x5]]
// CHECK-NEXT: OpStore %y [[mod3]]
    y %= x;

    float3 v3f;
    float4 v4f;

    // Vector swizzle
// CHECK-NEXT: [[ac0:%[0-9]+]] = OpAccessChain %_ptr_Function_float %v3f %int_2
// CHECK-NEXT: [[e0:%[0-9]+]] = OpLoad %float [[ac0]]
// CHECK-NEXT: [[ac1:%[0-9]+]] = OpAccessChain %_ptr_Function_float %v4f %int_0
// CHECK-NEXT: [[e1:%[0-9]+]] = OpLoad %float [[ac1]]
// CHECK-NEXT: [[mul4:%[0-9]+]] = OpFMul %float [[e1]] [[e0]]
// CHECK-NEXT: OpStore [[ac1]] [[mul4]]
    v4f.x *= v3f.z; // one element

// CHECK-NEXT: [[v3f0:%[0-9]+]] = OpLoad %v3float %v3f
// CHECK-NEXT: [[vs0:%[0-9]+]] = OpVectorShuffle %v2float [[v3f0]] [[v3f0]] 0 1
// CHECK-NEXT: [[v4f0:%[0-9]+]] = OpLoad %v4float %v4f
// CHECK-NEXT: [[vs1:%[0-9]+]] = OpVectorShuffle %v2float [[v4f0]] [[v4f0]] 2 3
// CHECK-NEXT: [[mul5:%[0-9]+]] = OpFMul %v2float [[vs1]] [[vs0]]
// CHECK-NEXT: [[v4f1:%[0-9]+]] = OpLoad %v4float %v4f
// CHECK-NEXT: [[vs2:%[0-9]+]] = OpVectorShuffle %v4float [[v4f1]] [[mul5]] 0 1 4 5
// CHECK-NEXT: OpStore %v4f [[vs2]]
    v4f.zw *= v3f.xy; // two elements

// CHECK-NEXT: [[v4f2:%[0-9]+]] = OpLoad %v4float %v4f
// CHECK-NEXT: [[vs3:%[0-9]+]] = OpVectorShuffle %v3float [[v4f2]] [[v4f2]] 0 1 2
// CHECK-NEXT: [[mul6:%[0-9]+]] = OpVectorTimesScalar %v3float [[vs3]] %float_4
// CHECK-NEXT: [[v4f3:%[0-9]+]] = OpLoad %v4float %v4f
// CHECK-NEXT: [[vs4:%[0-9]+]] = OpVectorShuffle %v4float [[v4f3]] [[mul6]] 4 5 6 3
// CHECK-NEXT: OpStore %v4f [[vs4]]
    v4f.xyz *= 4.0; // three elements (with scalar, should generate OpVectorTimesScalar)

    int4 v4i;

// CHECK-NEXT: [[v4i0:%[0-9]+]] = OpLoad %v4int %v4i
// CHECK-NEXT: [[vs5:%[0-9]+]] = OpVectorShuffle %v3int [[v4i0]] [[v4i0]] 0 1 2
// CHECK-NEXT: [[mul7:%[0-9]+]] = OpIMul %v3int [[vs5]] [[v3i4]]
// CHECK-NEXT: [[v4i1:%[0-9]+]] = OpLoad %v4int %v4i
// CHECK-NEXT: [[vs6:%[0-9]+]] = OpVectorShuffle %v4int [[v4i1]] [[mul7]] 4 5 6 3
// CHECK-NEXT: OpStore %v4i [[vs6]]
    v4i.xyz *= 4; // three elements (with scalar, but should not generate OpVectorTimesScalar)
}
