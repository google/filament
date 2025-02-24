// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

tbuffer MyTBuffer {
    float a = 1.0;
    float4 b = 2.0;
};

float main() : A {
    return 1.0;
}

// CHECK: :4:15: warning: tbuffer member initializer ignored since no Vulkan equivalent
// CHECK: :5:16: warning: tbuffer member initializer ignored since no Vulkan equivalent
