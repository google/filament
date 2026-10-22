#version 450

layout(binding = 0) uniform sampler2D uSamp;
uniform sampler2D SPIRV_Cross_CombineduTuS;

layout(location = 0) out vec4 FragColor;

void main()
{
    FragColor = texture(uSamp, vec2(0.5)) + texture(SPIRV_Cross_CombineduTuS, vec2(0.5));
}

