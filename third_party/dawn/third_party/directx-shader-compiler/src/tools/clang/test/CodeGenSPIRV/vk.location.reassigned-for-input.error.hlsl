// RUN: not %dxc -T vs_6_0 -E main -fcgl -spirv %s 2>&1 | FileCheck %s

struct S {
    [[vk::location(3)]] float a : A;
// CHECK: error: stage input location #3 already consumed by semantic 'M'
};

float main(
    [[vk::location(3)]]     float m : M,
    S s
) : R {
    return 1.0;
}
