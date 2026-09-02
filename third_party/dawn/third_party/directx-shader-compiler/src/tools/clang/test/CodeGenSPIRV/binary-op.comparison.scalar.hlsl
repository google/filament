// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

void main() {
    bool r;
    int a, b;
    uint i, j;
    float o, p;
    bool x, y;

// CHECK:      [[a0:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: [[b0:%[0-9]+]] = OpLoad %int %b
// CHECK-NEXT: {{%[0-9]+}} = OpSLessThan %bool [[a0]] [[b0]]
    r = a < b;
// CHECK:      [[a1:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: [[b1:%[0-9]+]] = OpLoad %int %b
// CHECK-NEXT: {{%[0-9]+}} = OpSLessThanEqual %bool [[a1]] [[b1]]
    r = a <= b;
// CHECK:      [[a2:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: [[b2:%[0-9]+]] = OpLoad %int %b
// CHECK-NEXT: {{%[0-9]+}} = OpSGreaterThan %bool [[a2]] [[b2]]
    r = a > b;
// CHECK:      [[a3:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: [[b3:%[0-9]+]] = OpLoad %int %b
// CHECK-NEXT: {{%[0-9]+}} = OpSGreaterThanEqual %bool [[a3]] [[b3]]
    r = a >= b;
// CHECK:      [[a4:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: [[b4:%[0-9]+]] = OpLoad %int %b
// CHECK-NEXT: {{%[0-9]+}} = OpIEqual %bool [[a4]] [[b4]]
    r = a == b;
// CHECK:      [[a5:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: [[b5:%[0-9]+]] = OpLoad %int %b
// CHECK-NEXT: {{%[0-9]+}} = OpINotEqual %bool [[a5]] [[b5]]
    r = a != b;

// CHECK:      [[i0:%[0-9]+]] = OpLoad %uint %i
// CHECK-NEXT: [[j0:%[0-9]+]] = OpLoad %uint %j
// CHECK-NEXT: {{%[0-9]+}} = OpULessThan %bool [[i0]] [[j0]]
    r = i < j;
// CHECK:      [[i1:%[0-9]+]] = OpLoad %uint %i
// CHECK-NEXT: [[j1:%[0-9]+]] = OpLoad %uint %j
// CHECK-NEXT: {{%[0-9]+}} = OpULessThanEqual %bool [[i1]] [[j1]]
    r = i <= j;
// CHECK:      [[i2:%[0-9]+]] = OpLoad %uint %i
// CHECK-NEXT: [[j2:%[0-9]+]] = OpLoad %uint %j
// CHECK-NEXT: {{%[0-9]+}} = OpUGreaterThan %bool [[i2]] [[j2]]
    r = i > j;
// CHECK:      [[i3:%[0-9]+]] = OpLoad %uint %i
// CHECK-NEXT: [[j3:%[0-9]+]] = OpLoad %uint %j
// CHECK-NEXT: {{%[0-9]+}} = OpUGreaterThanEqual %bool [[i3]] [[j3]]
    r = i >= j;
// CHECK:      [[i4:%[0-9]+]] = OpLoad %uint %i
// CHECK-NEXT: [[j4:%[0-9]+]] = OpLoad %uint %j
// CHECK-NEXT: {{%[0-9]+}} = OpIEqual %bool [[i4]] [[j4]]
    r = i == j;
// CHECK:      [[i5:%[0-9]+]] = OpLoad %uint %i
// CHECK-NEXT: [[j5:%[0-9]+]] = OpLoad %uint %j
// CHECK-NEXT: {{%[0-9]+}} = OpINotEqual %bool [[i5]] [[j5]]
    r = i != j;

// CHECK:      [[o0:%[0-9]+]] = OpLoad %float %o
// CHECK-NEXT: [[p0:%[0-9]+]] = OpLoad %float %p
// CHECK-NEXT: {{%[0-9]+}} = OpFOrdLessThan %bool [[o0]] [[p0]]
    r = o < p;
// CHECK:      [[o1:%[0-9]+]] = OpLoad %float %o
// CHECK-NEXT: [[p1:%[0-9]+]] = OpLoad %float %p
// CHECK-NEXT: {{%[0-9]+}} = OpFOrdLessThanEqual %bool [[o1]] [[p1]]
    r = o <= p;
// CHECK:      [[o2:%[0-9]+]] = OpLoad %float %o
// CHECK-NEXT: [[p2:%[0-9]+]] = OpLoad %float %p
// CHECK-NEXT: {{%[0-9]+}} = OpFOrdGreaterThan %bool [[o2]] [[p2]]
    r = o > p;
// CHECK:      [[o3:%[0-9]+]] = OpLoad %float %o
// CHECK-NEXT: [[p3:%[0-9]+]] = OpLoad %float %p
// CHECK-NEXT: {{%[0-9]+}} = OpFOrdGreaterThanEqual %bool [[o3]] [[p3]]
    r = o >= p;
// CHECK:      [[o4:%[0-9]+]] = OpLoad %float %o
// CHECK-NEXT: [[p4:%[0-9]+]] = OpLoad %float %p
// CHECK-NEXT: {{%[0-9]+}} = OpFOrdEqual %bool [[o4]] [[p4]]
    r = o == p;
// CHECK:      [[o5:%[0-9]+]] = OpLoad %float %o
// CHECK-NEXT: [[p5:%[0-9]+]] = OpLoad %float %p
// CHECK-NEXT: {{%[0-9]+}} = OpFOrdNotEqual %bool [[o5]] [[p5]]
    r = o != p;

// CHECK:      [[x0:%[0-9]+]] = OpLoad %bool %x
// CHECK-NEXT: [[y0:%[0-9]+]] = OpLoad %bool %y
// CHECK-NEXT: {{%[0-9]+}} = OpLogicalEqual %bool [[x0]] [[y0]]
    r = x == y;
// CHECK:      [[x1:%[0-9]+]] = OpLoad %bool %x
// CHECK-NEXT: [[y1:%[0-9]+]] = OpLoad %bool %y
// CHECK-NEXT: {{%[0-9]+}} = OpLogicalNotEqual %bool [[x1]] [[y1]]
    r = x != y;
}
