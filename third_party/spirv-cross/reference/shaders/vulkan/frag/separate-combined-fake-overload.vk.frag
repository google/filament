#version 450

layout(binding = 0) uniform sampler2D uSamp;
uniform sampler2D SPIRV_Cross_CombineduTuS;

layout(location = 0) out vec4 FragColor;

vec4 samp(sampler2D uSamp_1)
{
    return texture(uSamp_1, vec2(0.5));
}

vec4 samp_1(sampler2D SPIRV_Cross_CombinedTS)
{
    return texture(SPIRV_Cross_CombinedTS, vec2(0.5));
}

void main()
{
    FragColor = samp(uSamp) + samp_1(SPIRV_Cross_CombineduTuS);
}

