#version 450

struct _28
{
    vec4 _m0;
};

layout(binding = 0, std140) uniform _6_7
{
    vec4 _m0;
    float _m1;
    vec4 _m2;
} _7;

layout(binding = 2, std140) uniform _10_11
{
    vec3 _m0;
    vec3 _m1;
    float _m2;
    vec3 _m3;
    float _m4;
    vec3 _m5;
    float _m6;
    vec3 _m7;
    float _m8;
    vec3 _m9;
    float _m10;
    vec3 _m11;
    float _m12;
    vec2 _m13;
    vec2 _m14;
    vec3 _m15;
    float _m16;
    float _m17;
    float _m18;
    float _m19;
    float _m20;
    vec4 _m21;
    vec4 _m22;
    layout(row_major) mat4 _m23;
    vec4 _m24;
} _11;

layout(binding = 1, std140) uniform _18_19
{
    layout(row_major) mat4 _m0;
    layout(row_major) mat4 _m1;
    layout(row_major) mat4 _m2;
    layout(row_major) mat4 _m3;
    vec4 _m4;
    vec4 _m5;
    float _m6;
    float _m7;
    float _m8;
    float _m9;
    vec3 _m10;
    float _m11;
    vec3 _m12;
    float _m13;
    vec3 _m14;
    float _m15;
    vec3 _m16;
    float _m17;
    float _m18;
    float _m19;
    vec2 _m20;
    vec2 _m21;
    vec2 _m22;
    vec4 _m23;
    vec2 _m24;
    vec2 _m25;
    vec2 _m26;
    vec3 _m27;
    float _m28;
    float _m29;
    float _m30;
    float _m31;
    float _m32;
    vec2 _m33;
    float _m34;
    float _m35;
    vec3 _m36;
    layout(row_major) mat4 _m37[2];
    vec4 _m38[2];
} _19;

uniform sampler2D SPIRV_Cross_Combined;
uniform sampler2D SPIRV_Cross_Combined_1;
uniform sampler2D SPIRV_Cross_Combined_2;

layout(location = 0) out vec4 _5;

_28 _74;

void main()
{
    vec2 _82 = gl_FragCoord.xy * _19._m23.xy;
    vec4 _88 = _7._m2 * _7._m0.xyxy;
    vec2 _95 = _88.xy;
    vec2 _96 = _88.zw;
    vec2 _97 = clamp(_82 + (vec2(0.0, -2.0) * _7._m0.xy), _95, _96);
    vec3 _109 = _11._m5 * clamp(textureLod(SPIRV_Cross_Combined, _97, 0.0).w * _7._m1, 0.0, 1.0);
    vec4 _113 = textureLod(SPIRV_Cross_Combined_1, _97, 0.0);
    float _114 = _113.y;
    vec3 _129;
    if (_114 > 0.0)
    {
        _129 = _109 + (textureLod(SPIRV_Cross_Combined_2, _97, 0.0).xyz * clamp(_114 * _113.z, 0.0, 1.0));
    }
    else
    {
        _129 = _109;
    }
    vec3 _130 = _129 * 0.5;
    vec2 _144 = clamp(_82 + (vec2(-1.0) * _7._m0.xy), _95, _96);
    vec3 _156 = _11._m5 * clamp(textureLod(SPIRV_Cross_Combined, _144, 0.0).w * _7._m1, 0.0, 1.0);
    vec4 _160 = textureLod(SPIRV_Cross_Combined_1, _144, 0.0);
    float _161 = _160.y;
    vec3 _176;
    if (_161 > 0.0)
    {
        _176 = _156 + (textureLod(SPIRV_Cross_Combined_2, _144, 0.0).xyz * clamp(_161 * _160.z, 0.0, 1.0));
    }
    else
    {
        _176 = _156;
    }
    vec3 _177 = _176 * 0.5;
    vec2 _191 = clamp(_82 + (vec2(0.0, -1.0) * _7._m0.xy), _95, _96);
    vec3 _203 = _11._m5 * clamp(textureLod(SPIRV_Cross_Combined, _191, 0.0).w * _7._m1, 0.0, 1.0);
    vec4 _207 = textureLod(SPIRV_Cross_Combined_1, _191, 0.0);
    float _208 = _207.y;
    vec3 _223;
    if (_208 > 0.0)
    {
        _223 = _203 + (textureLod(SPIRV_Cross_Combined_2, _191, 0.0).xyz * clamp(_208 * _207.z, 0.0, 1.0));
    }
    else
    {
        _223 = _203;
    }
    vec3 _224 = _223 * 0.75;
    vec2 _238 = clamp(_82 + (vec2(1.0, -1.0) * _7._m0.xy), _95, _96);
    vec3 _250 = _11._m5 * clamp(textureLod(SPIRV_Cross_Combined, _238, 0.0).w * _7._m1, 0.0, 1.0);
    vec4 _254 = textureLod(SPIRV_Cross_Combined_1, _238, 0.0);
    float _255 = _254.y;
    vec3 _270;
    if (_255 > 0.0)
    {
        _270 = _250 + (textureLod(SPIRV_Cross_Combined_2, _238, 0.0).xyz * clamp(_255 * _254.z, 0.0, 1.0));
    }
    else
    {
        _270 = _250;
    }
    vec3 _271 = _270 * 0.5;
    vec2 _285 = clamp(_82 + (vec2(-2.0, 0.0) * _7._m0.xy), _95, _96);
    vec3 _297 = _11._m5 * clamp(textureLod(SPIRV_Cross_Combined, _285, 0.0).w * _7._m1, 0.0, 1.0);
    vec4 _301 = textureLod(SPIRV_Cross_Combined_1, _285, 0.0);
    float _302 = _301.y;
    vec3 _317;
    if (_302 > 0.0)
    {
        _317 = _297 + (textureLod(SPIRV_Cross_Combined_2, _285, 0.0).xyz * clamp(_302 * _301.z, 0.0, 1.0));
    }
    else
    {
        _317 = _297;
    }
    vec3 _318 = _317 * 0.5;
    vec2 _332 = clamp(_82 + (vec2(-1.0, 0.0) * _7._m0.xy), _95, _96);
    vec3 _344 = _11._m5 * clamp(textureLod(SPIRV_Cross_Combined, _332, 0.0).w * _7._m1, 0.0, 1.0);
    vec4 _348 = textureLod(SPIRV_Cross_Combined_1, _332, 0.0);
    float _349 = _348.y;
    vec3 _364;
    if (_349 > 0.0)
    {
        _364 = _344 + (textureLod(SPIRV_Cross_Combined_2, _332, 0.0).xyz * clamp(_349 * _348.z, 0.0, 1.0));
    }
    else
    {
        _364 = _344;
    }
    vec3 _365 = _364 * 0.75;
    vec2 _379 = clamp(_82, _95, _96);
    vec3 _391 = _11._m5 * clamp(textureLod(SPIRV_Cross_Combined, _379, 0.0).w * _7._m1, 0.0, 1.0);
    vec4 _395 = textureLod(SPIRV_Cross_Combined_1, _379, 0.0);
    float _396 = _395.y;
    vec3 _411;
    if (_396 > 0.0)
    {
        _411 = _391 + (textureLod(SPIRV_Cross_Combined_2, _379, 0.0).xyz * clamp(_396 * _395.z, 0.0, 1.0));
    }
    else
    {
        _411 = _391;
    }
    vec3 _412 = _411 * 1.0;
    vec2 _426 = clamp(_82 + (vec2(1.0, 0.0) * _7._m0.xy), _95, _96);
    vec3 _438 = _11._m5 * clamp(textureLod(SPIRV_Cross_Combined, _426, 0.0).w * _7._m1, 0.0, 1.0);
    vec4 _442 = textureLod(SPIRV_Cross_Combined_1, _426, 0.0);
    float _443 = _442.y;
    vec3 _458;
    if (_443 > 0.0)
    {
        _458 = _438 + (textureLod(SPIRV_Cross_Combined_2, _426, 0.0).xyz * clamp(_443 * _442.z, 0.0, 1.0));
    }
    else
    {
        _458 = _438;
    }
    vec3 _459 = _458 * 0.75;
    vec2 _473 = clamp(_82 + (vec2(2.0, 0.0) * _7._m0.xy), _95, _96);
    vec3 _485 = _11._m5 * clamp(textureLod(SPIRV_Cross_Combined, _473, 0.0).w * _7._m1, 0.0, 1.0);
    vec4 _489 = textureLod(SPIRV_Cross_Combined_1, _473, 0.0);
    float _490 = _489.y;
    vec3 _505;
    if (_490 > 0.0)
    {
        _505 = _485 + (textureLod(SPIRV_Cross_Combined_2, _473, 0.0).xyz * clamp(_490 * _489.z, 0.0, 1.0));
    }
    else
    {
        _505 = _485;
    }
    vec3 _506 = _505 * 0.5;
    vec2 _520 = clamp(_82 + (vec2(-1.0, 1.0) * _7._m0.xy), _95, _96);
    vec3 _532 = _11._m5 * clamp(textureLod(SPIRV_Cross_Combined, _520, 0.0).w * _7._m1, 0.0, 1.0);
    vec4 _536 = textureLod(SPIRV_Cross_Combined_1, _520, 0.0);
    float _537 = _536.y;
    vec3 _552;
    if (_537 > 0.0)
    {
        _552 = _532 + (textureLod(SPIRV_Cross_Combined_2, _520, 0.0).xyz * clamp(_537 * _536.z, 0.0, 1.0));
    }
    else
    {
        _552 = _532;
    }
    vec3 _553 = _552 * 0.5;
    vec2 _567 = clamp(_82 + (vec2(0.0, 1.0) * _7._m0.xy), _95, _96);
    vec3 _579 = _11._m5 * clamp(textureLod(SPIRV_Cross_Combined, _567, 0.0).w * _7._m1, 0.0, 1.0);
    vec4 _583 = textureLod(SPIRV_Cross_Combined_1, _567, 0.0);
    float _584 = _583.y;
    vec3 _599;
    if (_584 > 0.0)
    {
        _599 = _579 + (textureLod(SPIRV_Cross_Combined_2, _567, 0.0).xyz * clamp(_584 * _583.z, 0.0, 1.0));
    }
    else
    {
        _599 = _579;
    }
    vec3 _600 = _599 * 0.75;
    vec2 _614 = clamp(_82 + _7._m0.xy, _95, _96);
    vec3 _626 = _11._m5 * clamp(textureLod(SPIRV_Cross_Combined, _614, 0.0).w * _7._m1, 0.0, 1.0);
    vec4 _630 = textureLod(SPIRV_Cross_Combined_1, _614, 0.0);
    float _631 = _630.y;
    vec3 _646;
    if (_631 > 0.0)
    {
        _646 = _626 + (textureLod(SPIRV_Cross_Combined_2, _614, 0.0).xyz * clamp(_631 * _630.z, 0.0, 1.0));
    }
    else
    {
        _646 = _626;
    }
    vec3 _647 = _646 * 0.5;
    vec2 _661 = clamp(_82 + (vec2(0.0, 2.0) * _7._m0.xy), _95, _96);
    vec3 _673 = _11._m5 * clamp(textureLod(SPIRV_Cross_Combined, _661, 0.0).w * _7._m1, 0.0, 1.0);
    vec4 _677 = textureLod(SPIRV_Cross_Combined_1, _661, 0.0);
    float _678 = _677.y;
    vec3 _693;
    if (_678 > 0.0)
    {
        _693 = _673 + (textureLod(SPIRV_Cross_Combined_2, _661, 0.0).xyz * clamp(_678 * _677.z, 0.0, 1.0));
    }
    else
    {
        _693 = _673;
    }
    vec3 _702 = ((((((((((((_130.xyz + _177).xyz + _224).xyz + _271).xyz + _318).xyz + _365).xyz + _412).xyz + _459).xyz + _506).xyz + _553).xyz + _600).xyz + _647).xyz + (_693 * 0.5)).xyz * vec3(0.125);
    _28 _704 = _74;
    _704._m0 = vec4(_702.x, _702.y, _702.z, vec4(0.0).w);
    _28 _705 = _704;
    _705._m0.w = 1.0;
    _5 = _705._m0;
}

