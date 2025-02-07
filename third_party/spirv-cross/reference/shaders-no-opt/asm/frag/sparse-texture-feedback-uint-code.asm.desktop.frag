#version 450
#extension GL_ARB_sparse_texture2 : require

struct ResType
{
    uint _m0;
    vec4 _m1;
};

layout(binding = 0) uniform sampler2D uSamp;

layout(location = 0) in vec2 vUV;

void main()
{
    uint _30;
    vec4 _31;
    _30 = sparseTextureARB(uSamp, vUV, _31);
    ResType _24 = ResType(_30, _31);
    vec4 texel = _24._m1;
    bool ret = sparseTexelsResidentARB(int(_24._m0));
}

