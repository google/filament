// RUN: not %dxc -T vs_6_0 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

[[vk::constant_id(0)]]
const bool sc0 = true;

[[vk::constant_id(4)]]
const int sc4 = sc0 + sc0; // error

float main() : A {
    return 1.0;
}

// CHECK: :7:17: error: unsupported specialization constant initializer
