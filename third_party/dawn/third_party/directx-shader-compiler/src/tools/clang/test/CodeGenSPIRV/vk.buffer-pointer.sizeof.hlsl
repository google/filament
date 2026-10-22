// RUN: %dxc -T cs_6_0 -E main -fcgl %s -spirv | FileCheck %s

struct S {
    float a;
};

[numthreads(1, 1, 1)]
void main() {
    vk::BufferPointer<S> p = vk::BufferPointer<S>(0);
    // CHECK: %uint_8 = OpConstant %uint 8
    // CHECK: OpStore %size %uint_8
    uint size = sizeof(p);
}
