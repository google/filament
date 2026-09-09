// RUN: %dxc -T ps_6_0 -E main -O3  %s -spirv | FileCheck %s

struct PSInput
{
    float4 color : COLOR;
};

Texture2D bindless[];

sampler DummySampler;

// CHECK: [[src:%[0-9]+]] = OpAccessChain %_ptr_UniformConstant_type_2d_image %bindless %uint_4
// CHECK:                OpLoad %type_2d_image [[src]]

float4 SampleArray(Texture2D src[], uint index, float2 uv)
{
    return src[index].Sample(DummySampler, uv);
}

float4 main(PSInput input) : SV_TARGET
{
    return input.color * SampleArray(bindless, 4, float2(1,1));
}
