#version 450

struct Registers
{
    int index;
};

uniform Registers registers;

uniform sampler2D SPIRV_Cross_CombineduSamplerSPIRV_Cross_DummySampler[4];

layout(location = 0) out vec4 FragColor;

void main()
{
    FragColor = (texelFetch(SPIRV_Cross_CombineduSamplerSPIRV_Cross_DummySampler[registers.index], ivec2(10), 0) + texelFetch(SPIRV_Cross_CombineduSamplerSPIRV_Cross_DummySampler[registers.index], ivec2(4), 0)) + texelFetch(SPIRV_Cross_CombineduSamplerSPIRV_Cross_DummySampler[registers.index], ivec2(4), 0);
}

