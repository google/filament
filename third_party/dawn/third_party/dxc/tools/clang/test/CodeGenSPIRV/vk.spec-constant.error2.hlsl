// RUN: not %dxc -T vs_6_0 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

[[vk::constant_id(0)]]
const bool sc0 = true;

[[vk::constant_id(2)]]
const double sc2 = 42; // error

float main() : A {
    return 1.0;
}

// CHECK:  :7:1: error: unsupported specialization constant type
