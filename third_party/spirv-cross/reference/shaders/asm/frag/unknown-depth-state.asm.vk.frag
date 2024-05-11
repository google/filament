#version 450

layout(binding = 0) uniform sampler2DShadow uShadow;
uniform sampler2DShadow SPIRV_Cross_CombineduTextureuSampler;

layout(location = 0) in vec3 vUV;
layout(location = 0) out float FragColor;

float sample_combined()
{
    return texture(uShadow, vec3(vUV.xy, vUV.z));
}

float sample_separate()
{
    return texture(SPIRV_Cross_CombineduTextureuSampler, vec3(vUV.xy, vUV.z));
}

void main()
{
    FragColor = sample_combined() + sample_separate();
}

