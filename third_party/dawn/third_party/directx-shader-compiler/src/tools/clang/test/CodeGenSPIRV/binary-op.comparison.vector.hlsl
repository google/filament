// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

void main() {
// CHECK-LABEL: %bb_entry = OpLabel

    bool  r1;
    bool2 r2;
    bool3 r3;
    bool4 r4;

    int1  a, b;
    int2   i, j;
    uint3  m, n;
    float4 o, p;
    bool2  x, y;

// CHECK:      [[a0:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: [[b0:%[0-9]+]] = OpLoad %int %b
// CHECK-NEXT: {{%[0-9]+}} = OpSLessThan %bool [[a0]] [[b0]]
    r1 = a < b;
// CHECK:      [[a1:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: [[b1:%[0-9]+]] = OpLoad %int %b
// CHECK-NEXT: {{%[0-9]+}} = OpSLessThanEqual %bool [[a1]] [[b1]]
    r1 = a <= b;
// CHECK:      [[a2:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: [[b2:%[0-9]+]] = OpLoad %int %b
// CHECK-NEXT: {{%[0-9]+}} = OpSGreaterThan %bool [[a2]] [[b2]]
    r1 = a > b;
// CHECK:      [[a3:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: [[b3:%[0-9]+]] = OpLoad %int %b
// CHECK-NEXT: {{%[0-9]+}} = OpSGreaterThanEqual %bool [[a3]] [[b3]]
    r1 = a >= b;
// CHECK:      [[a4:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: [[b4:%[0-9]+]] = OpLoad %int %b
// CHECK-NEXT: {{%[0-9]+}} = OpIEqual %bool [[a4]] [[b4]]
    r1 = a == b;
// CHECK:      [[a5:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: [[b5:%[0-9]+]] = OpLoad %int %b
// CHECK-NEXT: {{%[0-9]+}} = OpINotEqual %bool [[a5]] [[b5]]
    r1 = a != b;

// CHECK:      [[i0:%[0-9]+]] = OpLoad %v2int %i
// CHECK-NEXT: [[j0:%[0-9]+]] = OpLoad %v2int %j
// CHECK-NEXT: {{%[0-9]+}} = OpSLessThan %v2bool [[i0]] [[j0]]
    r2 = i < j;
// CHECK:      [[i1:%[0-9]+]] = OpLoad %v2int %i
// CHECK-NEXT: [[j1:%[0-9]+]] = OpLoad %v2int %j
// CHECK-NEXT: {{%[0-9]+}} = OpSLessThanEqual %v2bool [[i1]] [[j1]]
    r2 = i <= j;
// CHECK:      [[i2:%[0-9]+]] = OpLoad %v2int %i
// CHECK-NEXT: [[j2:%[0-9]+]] = OpLoad %v2int %j
// CHECK-NEXT: {{%[0-9]+}} = OpSGreaterThan %v2bool [[i2]] [[j2]]
    r2 = i > j;
// CHECK:      [[i3:%[0-9]+]] = OpLoad %v2int %i
// CHECK-NEXT: [[j3:%[0-9]+]] = OpLoad %v2int %j
// CHECK-NEXT: {{%[0-9]+}} = OpSGreaterThanEqual %v2bool [[i3]] [[j3]]
    r2 = i >= j;
// CHECK:      [[i4:%[0-9]+]] = OpLoad %v2int %i
// CHECK-NEXT: [[j4:%[0-9]+]] = OpLoad %v2int %j
// CHECK-NEXT: {{%[0-9]+}} = OpIEqual %v2bool [[i4]] [[j4]]
    r2 = i == j;
// CHECK:      [[i5:%[0-9]+]] = OpLoad %v2int %i
// CHECK-NEXT: [[j5:%[0-9]+]] = OpLoad %v2int %j
// CHECK-NEXT: {{%[0-9]+}} = OpINotEqual %v2bool [[i5]] [[j5]]
    r2 = i != j;

// CHECK:      [[m0:%[0-9]+]] = OpLoad %v3uint %m
// CHECK-NEXT: [[n0:%[0-9]+]] = OpLoad %v3uint %n
// CHECK-NEXT: {{%[0-9]+}} = OpULessThan %v3bool [[m0]] [[n0]]
    r3 = m < n;
// CHECK:      [[m1:%[0-9]+]] = OpLoad %v3uint %m
// CHECK-NEXT: [[n1:%[0-9]+]] = OpLoad %v3uint %n
// CHECK-NEXT: {{%[0-9]+}} = OpULessThanEqual %v3bool [[m1]] [[n1]]
    r3 = m <= n;
// CHECK:      [[m2:%[0-9]+]] = OpLoad %v3uint %m
// CHECK-NEXT: [[n2:%[0-9]+]] = OpLoad %v3uint %n
// CHECK-NEXT: {{%[0-9]+}} = OpUGreaterThan %v3bool [[m2]] [[n2]]
    r3 = m > n;
// CHECK:      [[m3:%[0-9]+]] = OpLoad %v3uint %m
// CHECK-NEXT: [[n3:%[0-9]+]] = OpLoad %v3uint %n
// CHECK-NEXT: {{%[0-9]+}} = OpUGreaterThanEqual %v3bool [[m3]] [[n3]]
    r3 = m >= n;
// CHECK:      [[m4:%[0-9]+]] = OpLoad %v3uint %m
// CHECK-NEXT: [[n4:%[0-9]+]] = OpLoad %v3uint %n
// CHECK-NEXT: {{%[0-9]+}} = OpIEqual %v3bool [[m4]] [[n4]]
    r3 = m == n;
// CHECK:      [[m5:%[0-9]+]] = OpLoad %v3uint %m
// CHECK-NEXT: [[n5:%[0-9]+]] = OpLoad %v3uint %n
// CHECK-NEXT: {{%[0-9]+}} = OpINotEqual %v3bool [[m5]] [[n5]]
    r3 = m != n;

// CHECK:      [[o0:%[0-9]+]] = OpLoad %v4float %o
// CHECK-NEXT: [[p0:%[0-9]+]] = OpLoad %v4float %p
// CHECK-NEXT: {{%[0-9]+}} = OpFOrdLessThan %v4bool [[o0]] [[p0]]
    r4 = o < p;
// CHECK:      [[o1:%[0-9]+]] = OpLoad %v4float %o
// CHECK-NEXT: [[p1:%[0-9]+]] = OpLoad %v4float %p
// CHECK-NEXT: {{%[0-9]+}} = OpFOrdLessThanEqual %v4bool [[o1]] [[p1]]
    r4 = o <= p;
// CHECK:      [[o2:%[0-9]+]] = OpLoad %v4float %o
// CHECK-NEXT: [[p2:%[0-9]+]] = OpLoad %v4float %p
// CHECK-NEXT: {{%[0-9]+}} = OpFOrdGreaterThan %v4bool [[o2]] [[p2]]
    r4 = o > p;
// CHECK:      [[o3:%[0-9]+]] = OpLoad %v4float %o
// CHECK-NEXT: [[p3:%[0-9]+]] = OpLoad %v4float %p
// CHECK-NEXT: {{%[0-9]+}} = OpFOrdGreaterThanEqual %v4bool [[o3]] [[p3]]
    r4 = o >= p;
// CHECK:      [[o4:%[0-9]+]] = OpLoad %v4float %o
// CHECK-NEXT: [[p4:%[0-9]+]] = OpLoad %v4float %p
// CHECK-NEXT: {{%[0-9]+}} = OpFOrdEqual %v4bool [[o4]] [[p4]]
    r4 = o == p;
// CHECK:      [[o5:%[0-9]+]] = OpLoad %v4float %o
// CHECK-NEXT: [[p5:%[0-9]+]] = OpLoad %v4float %p
// CHECK-NEXT: {{%[0-9]+}} = OpFOrdNotEqual %v4bool [[o5]] [[p5]]
    r4 = o != p;

// CHECK:      [[x0:%[0-9]+]] = OpLoad %v2bool %x
// CHECK-NEXT: [[y0:%[0-9]+]] = OpLoad %v2bool %y
// CHECK-NEXT: {{%[0-9]+}} = OpLogicalEqual %v2bool [[x0]] [[y0]]
    r2 = x == y;
// CHECK:      [[x1:%[0-9]+]] = OpLoad %v2bool %x
// CHECK-NEXT: [[y1:%[0-9]+]] = OpLoad %v2bool %y
// CHECK-NEXT: {{%[0-9]+}} = OpLogicalNotEqual %v2bool [[x1]] [[y1]]
    r2 = x != y;
}
