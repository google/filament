#version 450
#extension GL_EXT_texture_shadow_lod : enable

layout(binding = 0) uniform sampler2DArrayShadow s2da;
layout(binding = 1) uniform samplerCubeArrayShadow sca;
layout(binding = 2) uniform samplerCubeShadow sc;

layout(location = 0) out float c;

layout(location = 0) in vec4 tc;

void main() {
    c = texture(s2da, tc, 0.0);
    c = texture(sca, tc, 0.0, 0.0);
    c = textureOffset(s2da, tc, ivec2(0.0), 0.0);
    c = textureLod(s2da, tc, 0.0);
    c = textureLod(sc, tc, 0.0);
    c = textureLod(sca, tc, 0.0, 0.0);
    c = textureLodOffset(s2da, tc, 0.0, ivec2(0.0));
}
