// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

void main() {
// CHECK-LABEL: %bb_entry = OpLabel
    int a, b, c, d;
    uint i, j;
    float o, p;

// CHECK:      [[a0:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: [[b0:%[0-9]+]] = OpLoad %int %b
// CHECK-NEXT: [[add0:%[0-9]+]] = OpIAdd %int [[b0]] [[a0]]
// CHECK-NEXT: OpStore %b [[add0]]
    b += a;
// CHECK-NEXT: [[i0:%[0-9]+]] = OpLoad %uint %i
// CHECK-NEXT: [[j0:%[0-9]+]] = OpLoad %uint %j
// CHECK-NEXT: [[add1:%[0-9]+]] = OpIAdd %uint [[j0]] [[i0]]
// CHECK-NEXT: OpStore %j [[add1]]
    j += i;
// CHECK-NEXT: [[o0:%[0-9]+]] = OpLoad %float %o
// CHECK-NEXT: [[p0:%[0-9]+]] = OpLoad %float %p
// CHECK-NEXT: [[add2:%[0-9]+]] = OpFAdd %float [[p0]] [[o0]]
// CHECK-NEXT: OpStore %p [[add2]]
    p += o;

// CHECK-NEXT: [[a1:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: [[b1:%[0-9]+]] = OpLoad %int %b
// CHECK-NEXT: [[sub0:%[0-9]+]] = OpISub %int [[b1]] [[a1]]
// CHECK-NEXT: OpStore %b [[sub0]]
    b -= a;
// CHECK-NEXT: [[i1:%[0-9]+]] = OpLoad %uint %i
// CHECK-NEXT: [[j1:%[0-9]+]] = OpLoad %uint %j
// CHECK-NEXT: [[sub1:%[0-9]+]] = OpISub %uint [[j1]] [[i1]]
// CHECK-NEXT: OpStore %j [[sub1]]
    j -= i;
// CHECK-NEXT: [[o1:%[0-9]+]] = OpLoad %float %o
// CHECK-NEXT: [[p1:%[0-9]+]] = OpLoad %float %p
// CHECK-NEXT: [[sub2:%[0-9]+]] = OpFSub %float [[p1]] [[o1]]
// CHECK-NEXT: OpStore %p [[sub2]]
    p -= o;

// CHECK-NEXT: [[a2:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: [[b2:%[0-9]+]] = OpLoad %int %b
// CHECK-NEXT: [[mul0:%[0-9]+]] = OpIMul %int [[b2]] [[a2]]
// CHECK-NEXT: OpStore %b [[mul0]]
    b *= a;
// CHECK-NEXT: [[i2:%[0-9]+]] = OpLoad %uint %i
// CHECK-NEXT: [[j2:%[0-9]+]] = OpLoad %uint %j
// CHECK-NEXT: [[mul1:%[0-9]+]] = OpIMul %uint [[j2]] [[i2]]
// CHECK-NEXT: OpStore %j [[mul1]]
    j *= i;
// CHECK-NEXT: [[o2:%[0-9]+]] = OpLoad %float %o
// CHECK-NEXT: [[p2:%[0-9]+]] = OpLoad %float %p
// CHECK-NEXT: [[mul2:%[0-9]+]] = OpFMul %float [[p2]] [[o2]]
// CHECK-NEXT: OpStore %p [[mul2]]
    p *= o;

// CHECK-NEXT: [[a3:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: [[b3:%[0-9]+]] = OpLoad %int %b
// CHECK-NEXT: [[div0:%[0-9]+]] = OpSDiv %int [[b3]] [[a3]]
// CHECK-NEXT: OpStore %b [[div0]]
    b /= a;
// CHECK-NEXT: [[i3:%[0-9]+]] = OpLoad %uint %i
// CHECK-NEXT: [[j3:%[0-9]+]] = OpLoad %uint %j
// CHECK-NEXT: [[div1:%[0-9]+]] = OpUDiv %uint [[j3]] [[i3]]
// CHECK-NEXT: OpStore %j [[div1]]
    j /= i;
// CHECK-NEXT: [[o3:%[0-9]+]] = OpLoad %float %o
// CHECK-NEXT: [[p3:%[0-9]+]] = OpLoad %float %p
// CHECK-NEXT: [[div2:%[0-9]+]] = OpFDiv %float [[p3]] [[o3]]
// CHECK-NEXT: OpStore %p [[div2]]
    p /= o;

// CHECK-NEXT: [[a4:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: [[b4:%[0-9]+]] = OpLoad %int %b
// CHECK-NEXT: [[mod0:%[0-9]+]] = OpSRem %int [[b4]] [[a4]]
// CHECK-NEXT: OpStore %b [[mod0]]
    b %= a;
// CHECK-NEXT: [[i4:%[0-9]+]] = OpLoad %uint %i
// CHECK-NEXT: [[j4:%[0-9]+]] = OpLoad %uint %j
// CHECK-NEXT: [[mod1:%[0-9]+]] = OpUMod %uint [[j4]] [[i4]]
// CHECK-NEXT: OpStore %j [[mod1]]
    j %= i;
// CHECK-NEXT: [[o4:%[0-9]+]] = OpLoad %float %o
// CHECK-NEXT: [[p4:%[0-9]+]] = OpLoad %float %p
// CHECK-NEXT: [[mod2:%[0-9]+]] = OpFRem %float [[p4]] [[o4]]
// CHECK-NEXT: OpStore %p [[mod2]]
    p %= o;

    // Spot check that we can use the result on both the left-hand side
    // and the right-hand side
// CHECK-NEXT: [[a5:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: [[b5:%[0-9]+]] = OpLoad %int %b
// CHECK-NEXT: [[sub3:%[0-9]+]] = OpISub %int [[b5]] [[a5]]
// CHECK-NEXT: OpStore %b [[sub3]]
// CHECK-NEXT: [[b6:%[0-9]+]] = OpLoad %int %b
// CHECK-NEXT: [[c0:%[0-9]+]] = OpLoad %int %c
// CHECK-NEXT: [[d0:%[0-9]+]] = OpLoad %int %d
// CHECK-NEXT: [[add3:%[0-9]+]] = OpIAdd %int [[d0]] [[c0]]
// CHECK-NEXT: OpStore %d [[add3]]
// CHECK-NEXT: OpStore %d [[b6]]
    (d += c) = (b -= a);
}
