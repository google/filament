// RUN: not %dxc -T vs_6_0 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

float main([[vk::location(5), vk::builtin("PointSize")]] int input : A) : B {
    return input;
}

// CHECK: :3:31: error: cannot use vk::builtin and vk::location together
// CHECK: :3:31: error: PointSize builtin must be of float type
// CHECK: :3:31: error: PointSize builtin cannot be used as VSIn
