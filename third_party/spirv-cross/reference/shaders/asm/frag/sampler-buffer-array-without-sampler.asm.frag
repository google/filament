#version 450

struct Registers
{
    int index;
};

uniform Registers registers;

uniform sampler2D SPIRV_Cross_CombineduSamplerSPIRV_Cross_DummySampler[4];

layout(location = 0) out vec4 FragColor;

vec4 sample_from_func(sampler2D SPIRV_Cross_CombineduSamplerSPIRV_Cross_DummySampler_1[4])
{
    return texelFetch(SPIRV_Cross_CombineduSamplerSPIRV_Cross_DummySampler_1[registers.index], ivec2(4), 0);
}

vec4 sample_one_from_func(sampler2D SPIRV_Cross_CombineduSamplerSPIRV_Cross_DummySampler_1)
{
    return texelFetch(SPIRV_Cross_CombineduSamplerSPIRV_Cross_DummySampler_1, ivec2(4), 0);
}

void main()
{
    FragColor = (texelFetch(SPIRV_Cross_CombineduSamplerSPIRV_Cross_DummySampler[registers.index], ivec2(10), 0) + sample_from_func(SPIRV_Cross_CombineduSamplerSPIRV_Cross_DummySampler)) + sample_one_from_func(SPIRV_Cross_CombineduSamplerSPIRV_Cross_DummySampler[registers.index]);
}

