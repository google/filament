// RUN: not %dxc -T vs_6_0 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

float main(
    [[vk::location(5)]] int   a: A,
                        float b: B
) : R { return 1.0; }

// CHECK:      error: partial explicit stage input location assignment via
// CHECK-SAME: vk::location(X)
// CHECK-SAME: unsupported
