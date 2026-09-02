// RUN: %dxc -T cs_6_0 -E main -O3  %s -spirv | FileCheck %s

//CHECK: OpTypeImage %float Buffer 2 0 0 2 Rgba16f
[[vk::image_format("rgba16f")]]
RWBuffer<float4> Buf;

//CHECK: OpTypeImage %float Buffer 2 0 0 2 R32f
[[vk::image_format("r32f")]]
RWBuffer<float4> Buf_r32f;

[[vk::image_format("rgba8snorm")]]
RWBuffer<float4> Buf_rgba8snorm;

[[vk::image_format("rg16f")]]
RWBuffer<float4> Buf_rg16f;

[[vk::image_format("r11g11b10f")]]
RWBuffer<float4> Buf_r11g11b10f;

[[vk::image_format("rgb10a2")]]
RWBuffer<float4> Buf_rgb10a2;

[[vk::image_format("rg8")]]
RWBuffer<float4> Buf_rg8;

[[vk::image_format("r8")]]
RWBuffer<float4> Buf_r8;

[[vk::image_format("rg16snorm")]]
RWBuffer<float4> Buf_rg16snorm;

[[vk::image_format("rgba32i")]]
RWBuffer<int4> Buf_rgba32i;

[[vk::image_format("rg8i")]]
RWBuffer<int2> Buf_rg8i;

[[vk::image_format("rgba16ui")]]
RWBuffer<uint4> Buf_rgba16ui;

[[vk::image_format("rgb10a2ui")]]
RWBuffer<uint4> Buf_rgb10a2ui;

[[vk::image_format("r64i")]]
RWBuffer<int64_t> Buf_r64i;

[[vk::image_format("r64ui")]]
RWBuffer<uint64_t> Buf_r64ui;

struct S {
    RWBuffer<float4> b;
};

float4 getVal(RWBuffer<float4> b) {
    return b[0];
}

float4 getValStruct(S s) {
    return s.b[1];
}

[numthreads(1, 1, 1)]
void main() {
    RWBuffer<float4> foo;

    foo = Buf;

    float4 test = getVal(foo);
    test += getVal(Buf_r32f);

    S s;
    s.b = Buf;
    test += getValStruct(s);

    S s2;
    s2.b = Buf_r32f;
    test += getValStruct(s2);

    RWBuffer<float4> var = Buf;
    RWBuffer<float4> var2 = Buf_r32f;
    test += var[2];
    test += var2[2];

    Buf[10] = test + 1;
}
