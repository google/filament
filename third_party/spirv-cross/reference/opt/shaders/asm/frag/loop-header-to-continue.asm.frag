#version 450

struct Params
{
    vec4 TextureSize;
    vec4 Params1;
    vec4 Params2;
    vec4 Params3;
    vec4 Params4;
    vec4 Bloom;
};

layout(binding = 1, std140) uniform CB1
{
    Params CB1;
} _8;

uniform sampler2D SPIRV_Cross_CombinedmapTexturemapSampler;

layout(location = 0) in vec2 IN_uv;
layout(location = 0) out vec4 _entryPointOutput;

void main()
{
    vec4 _49 = texture(SPIRV_Cross_CombinedmapTexturemapSampler, IN_uv);
    float _50 = _49.y;
    float _55;
    float _58;
    _55 = 0.0;
    _58 = 0.0;
    for (int _60 = -3; _60 <= 3; )
    {
        float _64 = float(_60);
        vec4 _72 = texture(SPIRV_Cross_CombinedmapTexturemapSampler, IN_uv + (vec2(0.0, _8.CB1.TextureSize.w) * _64));
        float _78 = exp(((-_64) * _64) * 0.2222220003604888916015625) * float(abs(_72.y - _50) < clamp(_50 * 0.06399999558925628662109375, 7.999999797903001308441162109375e-05, 0.008000000379979610443115234375));
        _55 += (_72.x * _78);
        _58 += _78;
        _60++;
        continue;
    }
    _entryPointOutput = vec4(_55 / _58, _50, 0.0, 1.0);
}

