// RUN: %dxc -T cs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpTypeImage %float Buffer 2 0 0 2 Rgba16f
[[vk::image_format("rgba16f")]]
RWBuffer<float4> RWBuf[2];

// CHECK: OpTypeImage %float Buffer 2 0 0 1 Rgba16ui
[[vk::image_format("rgba16ui")]]
Buffer<float4> Buf[2];

//CHECK: OpTypeImage %float 2D 2 0 0 2 Rgba16f
[[vk::image_format("rgba16f")]]
RWTexture2D<float4> Tex[2];

[numthreads(1, 1, 1)]
void main() {
}
