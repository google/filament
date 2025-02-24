// RUN: not %dxc -T vs_6_0 -E main -fcgl -spirv %s 2>&1 | FileCheck %s

struct T {
    [[vk::location(3)]] float i : A;
// CHECK: error: stage output location #3 already consumed by semantic 'M'
};

[[vk::location(3)]]
float main(
    [[vk::location(3)]]     float m : M,
    out T t
) : R {
    return 1.0;
}
