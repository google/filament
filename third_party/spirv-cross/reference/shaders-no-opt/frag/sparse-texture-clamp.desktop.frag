#version 450
#extension GL_ARB_sparse_texture2 : require
#extension GL_ARB_sparse_texture_clamp : require

struct ResType
{
    int _m0;
    vec4 _m1;
};

layout(binding = 0) uniform sampler2D uSamp;

layout(location = 0) in vec2 vUV;

void main()
{
    int _66;
    vec4 _67;
    _66 = sparseTextureClampARB(uSamp, vUV, 1.0, _67, 2.0);
    ResType _25 = ResType(_66, _67);
    vec4 texel = _25._m1;
    int code = _25._m0;
    texel = textureClampARB(uSamp, vUV, 1.0, 2.0);
    int _68;
    vec4 _69;
    _68 = sparseTextureOffsetClampARB(uSamp, vUV, ivec2(1, 2), 1.0, _69, 2.0);
    ResType _37 = ResType(_68, _69);
    texel = _37._m1;
    code = _37._m0;
    texel = textureOffsetClampARB(uSamp, vUV, ivec2(1, 2), 1.0, 2.0);
    int _70;
    vec4 _71;
    _70 = sparseTextureGradClampARB(uSamp, vUV, vec2(1.0), vec2(2.0), 1.0, _71);
    ResType _47 = ResType(_70, _71);
    texel = _47._m1;
    code = _47._m0;
    texel = textureGradClampARB(uSamp, vUV, vec2(1.0), vec2(2.0), 1.0);
    int _72;
    vec4 _73;
    _72 = sparseTextureGradOffsetClampARB(uSamp, vUV, vec2(1.0), vec2(2.0), ivec2(-1, -2), 1.0, _73);
    ResType _58 = ResType(_72, _73);
    texel = _58._m1;
    code = _58._m0;
    texel = textureGradOffsetClampARB(uSamp, vUV, vec2(1.0), vec2(2.0), ivec2(-1, -2), 1.0);
}

