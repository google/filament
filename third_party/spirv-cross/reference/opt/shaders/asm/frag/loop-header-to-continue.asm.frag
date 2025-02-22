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
} _12;

uniform sampler2D SPIRV_Cross_CombinedmapTexturemapSampler;

layout(location = 0) in vec2 IN_uv;
layout(location = 0) out vec4 _entryPointOutput;

void main()
{
    vec2 _180 = vec2(0.0, _12.CB1.TextureSize.w);
    vec4 _206 = texture(SPIRV_Cross_CombinedmapTexturemapSampler, IN_uv);
    float _207 = _206.y;
    float _211 = clamp(_207 * 0.06399999558925628662109375, 7.999999797903001308441162109375e-05, 0.008000000379979610443115234375);
    float _276;
    float _277;
    _276 = 0.0;
    _277 = 0.0;
    for (int _278 = -3; _278 <= 3; )
    {
        float _220 = float(_278);
        float _227 = exp(((-_220) * _220) * 0.2222220003604888916015625);
        vec4 _236 = texture(SPIRV_Cross_CombinedmapTexturemapSampler, IN_uv + (_180 * _220));
        float _245 = float(abs(_236.y - _207) < _211);
        _276 = fma(_236.x, _227 * _245, _276);
        _277 = fma(_227, _245, _277);
        _278++;
        continue;
    }
    _entryPointOutput = vec4(_276 / _277, _207, 0.0, 1.0);
}

