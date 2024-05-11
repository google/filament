#version 450

uniform samplerCubeShadow SPIRV_Cross_CombinedpointLightShadowMapshadowSamplerPCF;

layout(location = 0) out float _entryPointOutput;

void main()
{
    _entryPointOutput = textureGrad(SPIRV_Cross_CombinedpointLightShadowMapshadowSamplerPCF, vec4(vec4(0.100000001490116119384765625, 0.100000001490116119384765625, 0.100000001490116119384765625, 0.5).xyz, 0.5), vec3(0.0), vec3(0.0));
}

