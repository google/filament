#version 320 es

#extension GL_EXT_texture_shadow_lod : enable


uniform lowp sampler2DArrayShadow s2da;
uniform lowp samplerCubeArrayShadow sca;
uniform lowp samplerCubeShadow sc;

in lowp vec4 tc;
out lowp float c;
void main()
{
    c = texture(s2da, tc, 0.0);
    c = texture(sca, tc, 0.0, 0.0);
    c = textureOffset(s2da, tc, ivec2(0.0), 0.0);
    c = textureLod(s2da, tc, 0.0);
    c = textureLod(sc, tc, 0.0);
    c = textureLod(sca, tc, 0.0, 0.0);
    c = textureLodOffset(s2da, tc, 0.0, ivec2(0.0));

}
