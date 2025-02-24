// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

[[vk::location(123456)]]
float main() : A { return 1.0; }

// CHECK: OpDecorate [[var:%[a-zA-Z0-9_]+]] Location 123456
