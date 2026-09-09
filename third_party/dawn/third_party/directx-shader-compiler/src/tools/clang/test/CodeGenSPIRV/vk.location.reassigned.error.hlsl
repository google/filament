// RUN: not %dxc -T vs_6_0 -E main -fcgl  %s -spirv 2>&1 | FileCheck %s

[[vk::location(3)]]
float main(
    [[vk::location(3)]]     float m : M,
    [[vk::location(3)]]     float n : N,
// CHECK: error: stage input location #3 already consumed by semantic 'M'
    [[vk::location(3)]] out float x : X
// CHECK: error: stage output location #3 already consumed by semantic 'M'
) : R {
    return 1.0;
}
