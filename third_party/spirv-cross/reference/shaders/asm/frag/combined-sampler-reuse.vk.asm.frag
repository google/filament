#version 450

uniform sampler2D SPIRV_Cross_CombineduTexuSampler;

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec2 vUV;

void main()
{
    FragColor = texture(SPIRV_Cross_CombineduTexuSampler, vUV);
    FragColor += textureOffset(SPIRV_Cross_CombineduTexuSampler, vUV, ivec2(1));
}

