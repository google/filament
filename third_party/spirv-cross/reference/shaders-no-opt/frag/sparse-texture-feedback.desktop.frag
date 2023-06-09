#version 450
#extension GL_ARB_sparse_texture2 : require
#extension GL_ARB_sparse_texture_clamp : require

struct ResType
{
    int _m0;
    vec4 _m1;
};

layout(binding = 0) uniform sampler2D uSamp;
layout(binding = 1) uniform sampler2DMS uSampMS;
layout(binding = 2, rgba8) uniform readonly image2D uImage;
layout(binding = 3, rgba8) uniform readonly image2DMS uImageMS;

layout(location = 0) in vec2 vUV;

void main()
{
    int _144;
    vec4 _145;
    _144 = sparseTextureARB(uSamp, vUV, _145);
    ResType _24 = ResType(_144, _145);
    vec4 texel = _24._m1;
    bool ret = sparseTexelsResidentARB(_24._m0);
    int _146;
    vec4 _147;
    _146 = sparseTextureARB(uSamp, vUV, _147, 1.10000002384185791015625);
    ResType _31 = ResType(_146, _147);
    texel = _31._m1;
    ret = sparseTexelsResidentARB(_31._m0);
    int _148;
    vec4 _149;
    _148 = sparseTextureLodARB(uSamp, vUV, 1.0, _149);
    ResType _38 = ResType(_148, _149);
    texel = _38._m1;
    ret = sparseTexelsResidentARB(_38._m0);
    int _150;
    vec4 _151;
    _150 = sparseTextureOffsetARB(uSamp, vUV, ivec2(1), _151);
    ResType _47 = ResType(_150, _151);
    texel = _47._m1;
    ret = sparseTexelsResidentARB(_47._m0);
    int _152;
    vec4 _153;
    _152 = sparseTextureOffsetARB(uSamp, vUV, ivec2(2), _153, 0.5);
    ResType _56 = ResType(_152, _153);
    texel = _56._m1;
    ret = sparseTexelsResidentARB(_56._m0);
    int _154;
    vec4 _155;
    _154 = sparseTexelFetchARB(uSamp, ivec2(vUV), 1, _155);
    ResType _64 = ResType(_154, _155);
    texel = _64._m1;
    ret = sparseTexelsResidentARB(_64._m0);
    int _156;
    vec4 _157;
    _156 = sparseTexelFetchARB(uSampMS, ivec2(vUV), 2, _157);
    ResType _76 = ResType(_156, _157);
    texel = _76._m1;
    ret = sparseTexelsResidentARB(_76._m0);
    int _158;
    vec4 _159;
    _158 = sparseTexelFetchOffsetARB(uSamp, ivec2(vUV), 1, ivec2(2, 3), _159);
    ResType _86 = ResType(_158, _159);
    texel = _86._m1;
    ret = sparseTexelsResidentARB(_86._m0);
    int _160;
    vec4 _161;
    _160 = sparseTextureLodOffsetARB(uSamp, vUV, 1.5, ivec2(2, 3), _161);
    ResType _93 = ResType(_160, _161);
    texel = _93._m1;
    ret = sparseTexelsResidentARB(_93._m0);
    int _162;
    vec4 _163;
    _162 = sparseTextureGradARB(uSamp, vUV, vec2(1.0), vec2(3.0), _163);
    ResType _102 = ResType(_162, _163);
    texel = _102._m1;
    ret = sparseTexelsResidentARB(_102._m0);
    int _164;
    vec4 _165;
    _164 = sparseTextureGradOffsetARB(uSamp, vUV, vec2(1.0), vec2(3.0), ivec2(-2, -3), _165);
    ResType _111 = ResType(_164, _165);
    texel = _111._m1;
    ret = sparseTexelsResidentARB(_111._m0);
    int _166;
    vec4 _167;
    _166 = sparseTextureClampARB(uSamp, vUV, 4.0, _167);
    ResType _118 = ResType(_166, _167);
    texel = _118._m1;
    ret = sparseTexelsResidentARB(_118._m0);
    int _168;
    vec4 _169;
    _168 = sparseImageLoadARB(uImage, ivec2(vUV), _169);
    ResType _128 = ResType(_168, _169);
    texel = _128._m1;
    ret = sparseTexelsResidentARB(_128._m0);
    int _170;
    vec4 _171;
    _170 = sparseImageLoadARB(uImageMS, ivec2(vUV), 1, _171);
    ResType _138 = ResType(_170, _171);
    texel = _138._m1;
    ret = sparseTexelsResidentARB(_138._m0);
}

