// RUN: %dxc -T vs_6_0 -E main -Wno-vk-ignored-features -fcgl  %s -spirv  2>&1 | FileCheck %s

cbuffer MyCBuffer {
    float a = 1.0;
    float4 b = 2.0;
};

float gFloat = 3.0;

float main() : A {
    return float4(1, 2, 3, 4); // float4 -> float will trigger a warning
}

// CHECK-NOT: initializer
// CHECK: warning: implicit truncation of vector type
// CHECK-NOT: initializer
