// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct S {
    float x;
};

struct T {
    float y;
    S z;
};

void main() {
    int a, b, c;

// CHECK: [[b0:%[0-9]+]] = OpLoad %int %b
// CHECK-NEXT: OpStore %a [[b0]]
    a = b;
// CHECK-NEXT: [[c0:%[0-9]+]] = OpLoad %int %c
// CHECK-NEXT: OpStore %b [[c0]]
// CHECK-NEXT: OpStore %a [[c0]]
    a = b = c;

// CHECK-NEXT: [[a0:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: OpStore %a [[a0]]
    a = a;
// CHECK-NEXT: [[a1:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: OpStore %a [[a1]]
// CHECK-NEXT: OpStore %a [[a1]]
    a = a = a;

    T p, q;

// CHECK-NEXT: [[q:%[0-9]+]] = OpLoad %T %q
// CHECK-NEXT: OpStore %p [[q]]
    p = q;     // assign as a whole
// CHECK-NEXT: [[q1ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_S %q %int_1
// CHECK-NEXT: [[q1val:%[0-9]+]] = OpLoad %S [[q1ptr]]
// CHECK-NEXT: [[p1ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_S %p %int_1
// CHECK-NEXT: OpStore [[p1ptr]] [[q1val]]
    p.z = q.z; // assign nested struct
}
