#version 450

uniform sampler2D SPIRV_Cross_CombinedTexSPIRV_Cross_DummySampler;

layout(location = 0) flat in uvec3 in_var_TEXCOORD0;
layout(location = 0) out vec4 out_var_SV_Target0;

void main()
{
    out_var_SV_Target0 = texelFetch(SPIRV_Cross_CombinedTexSPIRV_Cross_DummySampler, ivec2(in_var_TEXCOORD0.xy), int(in_var_TEXCOORD0.z));
}

