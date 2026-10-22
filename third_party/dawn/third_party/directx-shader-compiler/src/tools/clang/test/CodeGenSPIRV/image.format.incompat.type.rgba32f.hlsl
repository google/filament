// RUN: not %dxc -T vs_6_0 -E main -fspv-target-env=vulkan1.1 -fcgl  %s -spirv 2>&1 | FileCheck %s

[[vk::image_format("rgba32f")]]
RWTexture1D   <int4>    t1 ;

void main() {}

// CHECK: error: The image format and the sampled type are not compatible.
// CHECK-NEXT: For the table of compatible types, see https://docs.vulkan.org/spec/latest/appendices/spirvenv.html#spirvenv-format-type-matching.
// CHECK-NEXT: RWTexture1D   <int4>    t1 ;