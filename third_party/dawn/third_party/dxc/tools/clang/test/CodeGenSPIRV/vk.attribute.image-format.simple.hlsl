// RUN: %dxc -T cs_6_0 -E main -O3  %s -spirv | FileCheck %s

//CHECK: OpTypeImage %float Buffer 2 0 0 2 Rgba16f
[[vk::image_format("rgba16f")]]
RWBuffer<float4> Buf;
//CHECK: OpTypeImage %float 2D 2 0 0 2 Rgba16f
[[vk::image_format("rgba16f")]]
RWTexture2D<float4> Tex;

[numthreads(1, 1, 1)]
void main() {
    Buf[0] = Tex[uint2(0, 0)];
    Tex[uint2(1, 0)] = Buf[1];
}
