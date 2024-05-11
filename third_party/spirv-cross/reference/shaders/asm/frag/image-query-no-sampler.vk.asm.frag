#version 450

uniform sampler2D SPIRV_Cross_CombineduSampler2DSPIRV_Cross_DummySampler;
uniform sampler2DMS SPIRV_Cross_CombineduSampler2DMSSPIRV_Cross_DummySampler;

void main()
{
    ivec2 b = textureSize(SPIRV_Cross_CombineduSampler2DSPIRV_Cross_DummySampler, 0);
    ivec2 c = textureSize(SPIRV_Cross_CombineduSampler2DMSSPIRV_Cross_DummySampler);
    int l1 = textureQueryLevels(SPIRV_Cross_CombineduSampler2DSPIRV_Cross_DummySampler);
    int s0 = textureSamples(SPIRV_Cross_CombineduSampler2DMSSPIRV_Cross_DummySampler);
}

