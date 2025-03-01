#version 450

uniform sampler2DArrayShadow SPIRV_Cross_CombinedShadowMapShadowSamplerPCF;

layout(location = 0) in vec2 texCoords;
layout(location = 1) in float cascadeIndex;
layout(location = 2) in float fragDepth;
layout(location = 0) out vec4 _entryPointOutput;

vec4 _main(vec2 texCoords_1, float cascadeIndex_1, float fragDepth_1)
{
    vec4 _39 = vec4(vec3(texCoords_1, cascadeIndex_1), fragDepth_1);
    float c = textureGrad(SPIRV_Cross_CombinedShadowMapShadowSamplerPCF, vec4(_39.xyz, _39.w), vec2(0.0), vec2(0.0));
    return vec4(c, c, c, c);
}

void main()
{
    vec2 texCoords_1 = texCoords;
    float cascadeIndex_1 = cascadeIndex;
    float fragDepth_1 = fragDepth;
    vec2 param = texCoords_1;
    float param_1 = cascadeIndex_1;
    float param_2 = fragDepth_1;
    _entryPointOutput = _main(param, param_1, param_2);
}

