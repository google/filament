// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

void main() {
    int a, b, c;
    uint i, j, k;
    float o, p, q;

// CHECK:      [[a0:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: [[b0:%[0-9]+]] = OpLoad %int %b
// CHECK-NEXT: [[c0:%[0-9]+]] = OpIAdd %int [[a0]] [[b0]]
// CHECK-NEXT: OpStore %c [[c0]]
    c = a + b;
// CHECK-NEXT: [[i0:%[0-9]+]] = OpLoad %uint %i
// CHECK-NEXT: [[j0:%[0-9]+]] = OpLoad %uint %j
// CHECK-NEXT: [[k0:%[0-9]+]] = OpIAdd %uint [[i0]] [[j0]]
// CHECK-NEXT: OpStore %k [[k0]]
    k = i + j;
// CHECK-NEXT: [[o0:%[0-9]+]] = OpLoad %float %o
// CHECK-NEXT: [[p0:%[0-9]+]] = OpLoad %float %p
// CHECK-NEXT: [[q0:%[0-9]+]] = OpFAdd %float [[o0]] [[p0]]
// CHECK-NEXT: OpStore %q [[q0]]
    q = o + p;

// CHECK-NEXT: [[a1:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: [[b1:%[0-9]+]] = OpLoad %int %b
// CHECK-NEXT: [[c1:%[0-9]+]] = OpISub %int [[a1]] [[b1]]
// CHECK-NEXT: OpStore %c [[c1]]
    c = a - b;
// CHECK-NEXT: [[i1:%[0-9]+]] = OpLoad %uint %i
// CHECK-NEXT: [[j1:%[0-9]+]] = OpLoad %uint %j
// CHECK-NEXT: [[k1:%[0-9]+]] = OpISub %uint [[i1]] [[j1]]
// CHECK-NEXT: OpStore %k [[k1]]
    k = i - j;
// CHECK-NEXT: [[o1:%[0-9]+]] = OpLoad %float %o
// CHECK-NEXT: [[p1:%[0-9]+]] = OpLoad %float %p
// CHECK-NEXT: [[q1:%[0-9]+]] = OpFSub %float [[o1]] [[p1]]
// CHECK-NEXT: OpStore %q [[q1]]
    q = o - p;

// CHECK-NEXT: [[a2:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: [[b2:%[0-9]+]] = OpLoad %int %b
// CHECK-NEXT: [[c2:%[0-9]+]] = OpIMul %int [[a2]] [[b2]]
// CHECK-NEXT: OpStore %c [[c2]]
    c = a * b;
// CHECK-NEXT: [[i2:%[0-9]+]] = OpLoad %uint %i
// CHECK-NEXT: [[j2:%[0-9]+]] = OpLoad %uint %j
// CHECK-NEXT: [[k2:%[0-9]+]] = OpIMul %uint [[i2]] [[j2]]
// CHECK-NEXT: OpStore %k [[k2]]
    k = i * j;
// CHECK-NEXT: [[o2:%[0-9]+]] = OpLoad %float %o
// CHECK-NEXT: [[p2:%[0-9]+]] = OpLoad %float %p
// CHECK-NEXT: [[q2:%[0-9]+]] = OpFMul %float [[o2]] [[p2]]
// CHECK-NEXT: OpStore %q [[q2]]
    q = o * p;

// CHECK-NEXT: [[a3:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: [[b3:%[0-9]+]] = OpLoad %int %b
// CHECK-NEXT: [[c3:%[0-9]+]] = OpSDiv %int [[a3]] [[b3]]
// CHECK-NEXT: OpStore %c [[c3]]
    c = a / b;
// CHECK-NEXT: [[i3:%[0-9]+]] = OpLoad %uint %i
// CHECK-NEXT: [[j3:%[0-9]+]] = OpLoad %uint %j
// CHECK-NEXT: [[k3:%[0-9]+]] = OpUDiv %uint [[i3]] [[j3]]
// CHECK-NEXT: OpStore %k [[k3]]
    k = i / j;
// CHECK-NEXT: [[o3:%[0-9]+]] = OpLoad %float %o
// CHECK-NEXT: [[p3:%[0-9]+]] = OpLoad %float %p
// CHECK-NEXT: [[q3:%[0-9]+]] = OpFDiv %float [[o3]] [[p3]]
// CHECK-NEXT: OpStore %q [[q3]]
    q = o / p;

// CHECK-NEXT: [[a4:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: [[b4:%[0-9]+]] = OpLoad %int %b
// CHECK-NEXT: [[c4:%[0-9]+]] = OpSRem %int [[a4]] [[b4]]
// CHECK-NEXT: OpStore %c [[c4]]
    c = a % b;
// CHECK-NEXT: [[i4:%[0-9]+]] = OpLoad %uint %i
// CHECK-NEXT: [[j4:%[0-9]+]] = OpLoad %uint %j
// CHECK-NEXT: [[k4:%[0-9]+]] = OpUMod %uint [[i4]] [[j4]]
// CHECK-NEXT: OpStore %k [[k4]]
    k = i % j;
// CHECK-NEXT: [[o4:%[0-9]+]] = OpLoad %float %o
// CHECK-NEXT: [[p4:%[0-9]+]] = OpLoad %float %p
// CHECK-NEXT: [[q4:%[0-9]+]] = OpFRem %float [[o4]] [[p4]]
// CHECK-NEXT: OpStore %q [[q4]]
    q = o % p;

// CHECK-NEXT: [[a5:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: [[b5:%[0-9]+]] = OpLoad %int %b
// CHECK-NEXT: [[a6:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: [[in0:%[0-9]+]] = OpIMul %int [[b5]] [[a6]]
// CHECK-NEXT: [[c5:%[0-9]+]] = OpLoad %int %c
// CHECK-NEXT: [[in1:%[0-9]+]] = OpSDiv %int [[in0]] [[c5]]
// CHECK-NEXT: [[b6:%[0-9]+]] = OpLoad %int %b
// CHECK-NEXT: [[in2:%[0-9]+]] = OpSRem %int [[in1]] [[b6]]
// CHECK-NEXT: [[in3:%[0-9]+]] = OpIAdd %int [[a5]] [[in2]]
// CHECK-NEXT: OpStore %c [[in3]]
    c = a + b * a / c % b;
// CHECK-NEXT: [[i5:%[0-9]+]] = OpLoad %uint %i
// CHECK-NEXT: [[j5:%[0-9]+]] = OpLoad %uint %j
// CHECK-NEXT: [[i6:%[0-9]+]] = OpLoad %uint %i
// CHECK-NEXT: [[in4:%[0-9]+]] = OpIMul %uint [[j5]] [[i6]]
// CHECK-NEXT: [[k5:%[0-9]+]] = OpLoad %uint %k
// CHECK-NEXT: [[in5:%[0-9]+]] = OpUDiv %uint [[in4]] [[k5]]
// CHECK-NEXT: [[j6:%[0-9]+]] = OpLoad %uint %j
// CHECK-NEXT: [[in6:%[0-9]+]] = OpUMod %uint [[in5]] [[j6]]
// CHECK-NEXT: [[in7:%[0-9]+]] = OpIAdd %uint [[i5]] [[in6]]
// CHECK-NEXT: OpStore %k [[in7]]
    k = i + j * i / k % j;
// CHECK-NEXT: [[o5:%[0-9]+]] = OpLoad %float %o
// CHECK-NEXT: [[p5:%[0-9]+]] = OpLoad %float %p
// CHECK-NEXT: [[o6:%[0-9]+]] = OpLoad %float %o
// CHECK-NEXT: [[in8:%[0-9]+]] = OpFMul %float [[p5]] [[o6]]
// CHECK-NEXT: [[q5:%[0-9]+]] = OpLoad %float %q
// CHECK-NEXT: [[in9:%[0-9]+]] = OpFDiv %float [[in8]] [[q5]]
// CHECK-NEXT: [[p6:%[0-9]+]] = OpLoad %float %p
// CHECK-NEXT: [[in10:%[0-9]+]] = OpFRem %float [[in9]] [[p6]]
// CHECK-NEXT: [[in11:%[0-9]+]] = OpFAdd %float [[o5]] [[in10]]
// CHECK-NEXT: OpStore %q [[in11]]
    q = o + p * o / q % p;
}
