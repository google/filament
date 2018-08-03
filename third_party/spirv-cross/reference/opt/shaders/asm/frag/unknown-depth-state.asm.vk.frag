#version 450

layout(binding = 0) uniform sampler2DShadow uShadow;
uniform sampler2DShadow SPIRV_Cross_CombineduTextureuSampler;

layout(location = 0) in vec3 vUV;
layout(location = 0) out float FragColor;

void main()
{
    FragColor = texture(uShadow, vec3(vUV.xy, vUV.z)) + texture(SPIRV_Cross_CombineduTextureuSampler, vec3(vUV.xy, vUV.z));
}

