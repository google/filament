// RUN: not %dxc -T vs_6_0 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

struct S {
    float f;
};

[[vk::shader_record_ext, vk::binding(6)]]
ConstantBuffer<S> recordBuf;

float main() : A {
    return 1.0;
}

// CHECK: :7:3: error: vk::shader_record_ext attribute cannot be used together with vk::binding attribute
