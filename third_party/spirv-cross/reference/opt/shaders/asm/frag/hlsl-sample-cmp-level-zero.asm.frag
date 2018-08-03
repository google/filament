#version 450

uniform sampler2DArrayShadow SPIRV_Cross_CombinedShadowMapShadowSamplerPCF;

layout(location = 0) in vec2 texCoords;
layout(location = 1) in float cascadeIndex;
layout(location = 2) in float fragDepth;
layout(location = 0) out vec4 _entryPointOutput;

void main()
{
    _entryPointOutput = vec4(textureGrad(SPIRV_Cross_CombinedShadowMapShadowSamplerPCF, vec4(vec4(texCoords, cascadeIndex, fragDepth).xyz, fragDepth), vec2(0.0), vec2(0.0)));
}

