// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

void main() {
// CHECK-LABEL: %bb_entry = OpLabel

    int1 a, b, c;
    int2 i, j, k;
    uint3 o, p, q;
    float4 x, y, z;

// CHECK:      [[a0:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: [[b0:%[0-9]+]] = OpLoad %int %b
// CHECK-NEXT: [[add0:%[0-9]+]] = OpIAdd %int [[a0]] [[b0]]
// CHECK-NEXT: OpStore %c [[add0]]
    c = a + b;
// CHECK-NEXT: [[i0:%[0-9]+]] = OpLoad %v2int %i
// CHECK-NEXT: [[j0:%[0-9]+]] = OpLoad %v2int %j
// CHECK-NEXT: [[add1:%[0-9]+]] = OpIAdd %v2int [[i0]] [[j0]]
// CHECK-NEXT: OpStore %k [[add1]]
    k = i + j;
// CHECK-NEXT: [[o0:%[0-9]+]] = OpLoad %v3uint %o
// CHECK-NEXT: [[p0:%[0-9]+]] = OpLoad %v3uint %p
// CHECK-NEXT: [[add2:%[0-9]+]] = OpIAdd %v3uint [[o0]] [[p0]]
// CHECK-NEXT: OpStore %q [[add2]]
    q = o + p;
// CHECK-NEXT: [[x0:%[0-9]+]] = OpLoad %v4float %x
// CHECK-NEXT: [[z0:%[0-9]+]] = OpLoad %v4float %y
// CHECK-NEXT: [[add3:%[0-9]+]] = OpFAdd %v4float [[x0]] [[z0]]
// CHECK-NEXT: OpStore %z [[add3]]
    z = x + y;

// CHECK-NEXT: [[a1:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: [[b1:%[0-9]+]] = OpLoad %int %b
// CHECK-NEXT: [[sub0:%[0-9]+]] = OpISub %int [[a1]] [[b1]]
// CHECK-NEXT: OpStore %c [[sub0]]
    c = a - b;
// CHECK-NEXT: [[i1:%[0-9]+]] = OpLoad %v2int %i
// CHECK-NEXT: [[j1:%[0-9]+]] = OpLoad %v2int %j
// CHECK-NEXT: [[sub1:%[0-9]+]] = OpISub %v2int [[i1]] [[j1]]
// CHECK-NEXT: OpStore %k [[sub1]]
    k = i - j;
// CHECK-NEXT: [[o1:%[0-9]+]] = OpLoad %v3uint %o
// CHECK-NEXT: [[p1:%[0-9]+]] = OpLoad %v3uint %p
// CHECK-NEXT: [[sub2:%[0-9]+]] = OpISub %v3uint [[o1]] [[p1]]
// CHECK-NEXT: OpStore %q [[sub2]]
    q = o - p;
// CHECK-NEXT: [[x1:%[0-9]+]] = OpLoad %v4float %x
// CHECK-NEXT: [[y1:%[0-9]+]] = OpLoad %v4float %y
// CHECK-NEXT: [[sub3:%[0-9]+]] = OpFSub %v4float [[x1]] [[y1]]
// CHECK-NEXT: OpStore %z [[sub3]]
    z = x - y;

// CHECK-NEXT: [[a2:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: [[b2:%[0-9]+]] = OpLoad %int %b
// CHECK-NEXT: [[mul0:%[0-9]+]] = OpIMul %int [[a2]] [[b2]]
// CHECK-NEXT: OpStore %c [[mul0]]
    c = a * b;
// CHECK-NEXT: [[i2:%[0-9]+]] = OpLoad %v2int %i
// CHECK-NEXT: [[j2:%[0-9]+]] = OpLoad %v2int %j
// CHECK-NEXT: [[mul1:%[0-9]+]] = OpIMul %v2int [[i2]] [[j2]]
// CHECK-NEXT: OpStore %k [[mul1]]
    k = i * j;
// CHECK-NEXT: [[o2:%[0-9]+]] = OpLoad %v3uint %o
// CHECK-NEXT: [[p2:%[0-9]+]] = OpLoad %v3uint %p
// CHECK-NEXT: [[mul2:%[0-9]+]] = OpIMul %v3uint [[o2]] [[p2]]
// CHECK-NEXT: OpStore %q [[mul2]]
    q = o * p;
// CHECK-NEXT: [[x2:%[0-9]+]] = OpLoad %v4float %x
// CHECK-NEXT: [[y2:%[0-9]+]] = OpLoad %v4float %y
// CHECK-NEXT: [[mul3:%[0-9]+]] = OpFMul %v4float [[x2]] [[y2]]
// CHECK-NEXT: OpStore %z [[mul3]]
    z = x * y;

// CHECK-NEXT: [[a4:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: [[b4:%[0-9]+]] = OpLoad %int %b
// CHECK-NEXT: [[div0:%[0-9]+]] = OpSDiv %int [[a4]] [[b4]]
// CHECK-NEXT: OpStore %c [[div0]]
    c = a / b;
// CHECK-NEXT: [[i4:%[0-9]+]] = OpLoad %v2int %i
// CHECK-NEXT: [[j4:%[0-9]+]] = OpLoad %v2int %j
// CHECK-NEXT: [[div1:%[0-9]+]] = OpSDiv %v2int [[i4]] [[j4]]
// CHECK-NEXT: OpStore %k [[div1]]
    k = i / j;
// CHECK-NEXT: [[o4:%[0-9]+]] = OpLoad %v3uint %o
// CHECK-NEXT: [[p4:%[0-9]+]] = OpLoad %v3uint %p
// CHECK-NEXT: [[div2:%[0-9]+]] = OpUDiv %v3uint [[o4]] [[p4]]
// CHECK-NEXT: OpStore %q [[div2]]
    q = o / p;
// CHECK-NEXT: [[x4:%[0-9]+]] = OpLoad %v4float %x
// CHECK-NEXT: [[y4:%[0-9]+]] = OpLoad %v4float %y
// CHECK-NEXT: [[div3:%[0-9]+]] = OpFDiv %v4float [[x4]] [[y4]]
// CHECK-NEXT: OpStore %z [[div3]]
    z = x / y;

// CHECK-NEXT: [[a5:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: [[b5:%[0-9]+]] = OpLoad %int %b
// CHECK-NEXT: [[mod0:%[0-9]+]] = OpSRem %int [[a5]] [[b5]]
// CHECK-NEXT: OpStore %c [[mod0]]
    c = a % b;
// CHECK-NEXT: [[i5:%[0-9]+]] = OpLoad %v2int %i
// CHECK-NEXT: [[j5:%[0-9]+]] = OpLoad %v2int %j
// CHECK-NEXT: [[mod1:%[0-9]+]] = OpSRem %v2int [[i5]] [[j5]]
// CHECK-NEXT: OpStore %k [[mod1]]
    k = i % j;
// CHECK-NEXT: [[o5:%[0-9]+]] = OpLoad %v3uint %o
// CHECK-NEXT: [[p5:%[0-9]+]] = OpLoad %v3uint %p
// CHECK-NEXT: [[mod2:%[0-9]+]] = OpUMod %v3uint [[o5]] [[p5]]
// CHECK-NEXT: OpStore %q [[mod2]]
    q = o % p;
// CHECK-NEXT: [[x5:%[0-9]+]] = OpLoad %v4float %x
// CHECK-NEXT: [[y5:%[0-9]+]] = OpLoad %v4float %y
// CHECK-NEXT: [[mod3:%[0-9]+]] = OpFRem %v4float [[x5]] [[y5]]
// CHECK-NEXT: OpStore %z [[mod3]]
    z = x % y;
}
