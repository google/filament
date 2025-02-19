#version 450

uniform samplerCubeShadow SPIRV_Cross_CombinedpointLightShadowMapshadowSamplerPCF;

layout(location = 0) out float _entryPointOutput;

float _main()
{
    vec4 _29 = vec4(0.100000001490116119384765625, 0.100000001490116119384765625, 0.100000001490116119384765625, 0.5);
    return textureGrad(SPIRV_Cross_CombinedpointLightShadowMapshadowSamplerPCF, vec4(_29.xyz, _29.w), vec3(0.0), vec3(0.0));
}

void main()
{
    _entryPointOutput = _main();
}

