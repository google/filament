#version 450

uniform sampler2D SPIRV_Cross_Combinedg_Textureg_Sampler;
uniform sampler2DShadow SPIRV_Cross_Combinedg_Textureg_CompareSampler;

layout(location = 0) in vec2 in_var_TEXCOORD0;
layout(location = 0) out float out_var_SV_Target;

void main()
{
    out_var_SV_Target = texture(SPIRV_Cross_Combinedg_Textureg_Sampler, in_var_TEXCOORD0).x + textureLod(SPIRV_Cross_Combinedg_Textureg_CompareSampler, vec3(in_var_TEXCOORD0, 0.5), 0.0);
}

