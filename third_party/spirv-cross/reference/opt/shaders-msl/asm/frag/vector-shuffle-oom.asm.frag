#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct _28
{
    float4 _m0;
};

struct _6
{
    float4 _m0;
    float _m1;
    float4 _m2;
};

struct _10
{
    float3 _m0;
    packed_float3 _m1;
    float _m2;
    packed_float3 _m3;
    float _m4;
    packed_float3 _m5;
    float _m6;
    packed_float3 _m7;
    float _m8;
    packed_float3 _m9;
    float _m10;
    packed_float3 _m11;
    float _m12;
    float2 _m13;
    float2 _m14;
    packed_float3 _m15;
    float _m16;
    float _m17;
    float _m18;
    float _m19;
    float _m20;
    float4 _m21;
    float4 _m22;
    float4x4 _m23;
    float4 _m24;
};

struct _18
{
    float4x4 _m0;
    float4x4 _m1;
    float4x4 _m2;
    float4x4 _m3;
    float4 _m4;
    float4 _m5;
    float _m6;
    float _m7;
    float _m8;
    float _m9;
    packed_float3 _m10;
    float _m11;
    packed_float3 _m12;
    float _m13;
    packed_float3 _m14;
    float _m15;
    packed_float3 _m16;
    float _m17;
    float _m18;
    float _m19;
    float2 _m20;
    float2 _m21;
    float2 _m22;
    float4 _m23;
    float2 _m24;
    float2 _m25;
    float2 _m26;
    char _m27_pad[8];
    packed_float3 _m27;
    float _m28;
    float _m29;
    float _m30;
    float _m31;
    float _m32;
    float2 _m33;
    float _m34;
    float _m35;
    float3 _m36;
    float4x4 _m37[2];
    float4 _m38[2];
};

constant _28 _74 = {};

struct main0_out
{
    float4 m_5 [[color(0)]];
};

fragment main0_out main0(constant _6& _7 [[buffer(0)]], constant _10& _11 [[buffer(1)]], constant _18& _19 [[buffer(2)]], texture2d<float> _8 [[texture(0)]], texture2d<float> _12 [[texture(1)]], texture2d<float> _14 [[texture(2)]], sampler _9 [[sampler(0)]], sampler _13 [[sampler(1)]], sampler _15 [[sampler(2)]], float4 gl_FragCoord [[position]])
{
    main0_out out = {};
    float2 _82 = gl_FragCoord.xy * _19._m23.xy;
    float4 _88 = _7._m2 * _7._m0.xyxy;
    float2 _95 = _88.xy;
    float2 _96 = _88.zw;
    float2 _97 = fast::clamp(_82 + (float2(0.0, -2.0) * _7._m0.xy), _95, _96);
    float3 _109 = float3(_11._m5) * fast::clamp(_8.sample(_9, _97, level(0.0)).w * _7._m1, 0.0, 1.0);
    float4 _113 = _12.sample(_13, _97, level(0.0));
    float _114 = _113.y;
    float3 _129;
    if (_114 > 0.0)
    {
        _129 = _109 + (_14.sample(_15, _97, level(0.0)).xyz * fast::clamp(_114 * _113.z, 0.0, 1.0));
    }
    else
    {
        _129 = _109;
    }
    float2 _144 = fast::clamp(_82 + (float2(-1.0) * _7._m0.xy), _95, _96);
    float3 _156 = float3(_11._m5) * fast::clamp(_8.sample(_9, _144, level(0.0)).w * _7._m1, 0.0, 1.0);
    float4 _160 = _12.sample(_13, _144, level(0.0));
    float _161 = _160.y;
    float3 _176;
    if (_161 > 0.0)
    {
        _176 = _156 + (_14.sample(_15, _144, level(0.0)).xyz * fast::clamp(_161 * _160.z, 0.0, 1.0));
    }
    else
    {
        _176 = _156;
    }
    float2 _191 = fast::clamp(_82 + (float2(0.0, -1.0) * _7._m0.xy), _95, _96);
    float3 _203 = float3(_11._m5) * fast::clamp(_8.sample(_9, _191, level(0.0)).w * _7._m1, 0.0, 1.0);
    float4 _207 = _12.sample(_13, _191, level(0.0));
    float _208 = _207.y;
    float3 _223;
    if (_208 > 0.0)
    {
        _223 = _203 + (_14.sample(_15, _191, level(0.0)).xyz * fast::clamp(_208 * _207.z, 0.0, 1.0));
    }
    else
    {
        _223 = _203;
    }
    float2 _238 = fast::clamp(_82 + (float2(1.0, -1.0) * _7._m0.xy), _95, _96);
    float3 _250 = float3(_11._m5) * fast::clamp(_8.sample(_9, _238, level(0.0)).w * _7._m1, 0.0, 1.0);
    float4 _254 = _12.sample(_13, _238, level(0.0));
    float _255 = _254.y;
    float3 _270;
    if (_255 > 0.0)
    {
        _270 = _250 + (_14.sample(_15, _238, level(0.0)).xyz * fast::clamp(_255 * _254.z, 0.0, 1.0));
    }
    else
    {
        _270 = _250;
    }
    float2 _285 = fast::clamp(_82 + (float2(-2.0, 0.0) * _7._m0.xy), _95, _96);
    float3 _297 = float3(_11._m5) * fast::clamp(_8.sample(_9, _285, level(0.0)).w * _7._m1, 0.0, 1.0);
    float4 _301 = _12.sample(_13, _285, level(0.0));
    float _302 = _301.y;
    float3 _317;
    if (_302 > 0.0)
    {
        _317 = _297 + (_14.sample(_15, _285, level(0.0)).xyz * fast::clamp(_302 * _301.z, 0.0, 1.0));
    }
    else
    {
        _317 = _297;
    }
    float2 _332 = fast::clamp(_82 + (float2(-1.0, 0.0) * _7._m0.xy), _95, _96);
    float3 _344 = float3(_11._m5) * fast::clamp(_8.sample(_9, _332, level(0.0)).w * _7._m1, 0.0, 1.0);
    float4 _348 = _12.sample(_13, _332, level(0.0));
    float _349 = _348.y;
    float3 _364;
    if (_349 > 0.0)
    {
        _364 = _344 + (_14.sample(_15, _332, level(0.0)).xyz * fast::clamp(_349 * _348.z, 0.0, 1.0));
    }
    else
    {
        _364 = _344;
    }
    float2 _379 = fast::clamp(_82, _95, _96);
    float3 _391 = float3(_11._m5) * fast::clamp(_8.sample(_9, _379, level(0.0)).w * _7._m1, 0.0, 1.0);
    float4 _395 = _12.sample(_13, _379, level(0.0));
    float _396 = _395.y;
    float3 _411;
    if (_396 > 0.0)
    {
        _411 = _391 + (_14.sample(_15, _379, level(0.0)).xyz * fast::clamp(_396 * _395.z, 0.0, 1.0));
    }
    else
    {
        _411 = _391;
    }
    float2 _426 = fast::clamp(_82 + (float2(1.0, 0.0) * _7._m0.xy), _95, _96);
    float3 _438 = float3(_11._m5) * fast::clamp(_8.sample(_9, _426, level(0.0)).w * _7._m1, 0.0, 1.0);
    float4 _442 = _12.sample(_13, _426, level(0.0));
    float _443 = _442.y;
    float3 _458;
    if (_443 > 0.0)
    {
        _458 = _438 + (_14.sample(_15, _426, level(0.0)).xyz * fast::clamp(_443 * _442.z, 0.0, 1.0));
    }
    else
    {
        _458 = _438;
    }
    float2 _473 = fast::clamp(_82 + (float2(2.0, 0.0) * _7._m0.xy), _95, _96);
    float3 _485 = float3(_11._m5) * fast::clamp(_8.sample(_9, _473, level(0.0)).w * _7._m1, 0.0, 1.0);
    float4 _489 = _12.sample(_13, _473, level(0.0));
    float _490 = _489.y;
    float3 _505;
    if (_490 > 0.0)
    {
        _505 = _485 + (_14.sample(_15, _473, level(0.0)).xyz * fast::clamp(_490 * _489.z, 0.0, 1.0));
    }
    else
    {
        _505 = _485;
    }
    float2 _520 = fast::clamp(_82 + (float2(-1.0, 1.0) * _7._m0.xy), _95, _96);
    float3 _532 = float3(_11._m5) * fast::clamp(_8.sample(_9, _520, level(0.0)).w * _7._m1, 0.0, 1.0);
    float4 _536 = _12.sample(_13, _520, level(0.0));
    float _537 = _536.y;
    float3 _552;
    if (_537 > 0.0)
    {
        _552 = _532 + (_14.sample(_15, _520, level(0.0)).xyz * fast::clamp(_537 * _536.z, 0.0, 1.0));
    }
    else
    {
        _552 = _532;
    }
    float2 _567 = fast::clamp(_82 + (float2(0.0, 1.0) * _7._m0.xy), _95, _96);
    float3 _579 = float3(_11._m5) * fast::clamp(_8.sample(_9, _567, level(0.0)).w * _7._m1, 0.0, 1.0);
    float4 _583 = _12.sample(_13, _567, level(0.0));
    float _584 = _583.y;
    float3 _599;
    if (_584 > 0.0)
    {
        _599 = _579 + (_14.sample(_15, _567, level(0.0)).xyz * fast::clamp(_584 * _583.z, 0.0, 1.0));
    }
    else
    {
        _599 = _579;
    }
    float2 _614 = fast::clamp(_82 + _7._m0.xy, _95, _96);
    float3 _626 = float3(_11._m5) * fast::clamp(_8.sample(_9, _614, level(0.0)).w * _7._m1, 0.0, 1.0);
    float4 _630 = _12.sample(_13, _614, level(0.0));
    float _631 = _630.y;
    float3 _646;
    if (_631 > 0.0)
    {
        _646 = _626 + (_14.sample(_15, _614, level(0.0)).xyz * fast::clamp(_631 * _630.z, 0.0, 1.0));
    }
    else
    {
        _646 = _626;
    }
    float2 _661 = fast::clamp(_82 + (float2(0.0, 2.0) * _7._m0.xy), _95, _96);
    float3 _673 = float3(_11._m5) * fast::clamp(_8.sample(_9, _661, level(0.0)).w * _7._m1, 0.0, 1.0);
    float4 _677 = _12.sample(_13, _661, level(0.0));
    float _678 = _677.y;
    float3 _693;
    if (_678 > 0.0)
    {
        _693 = _673 + (_14.sample(_15, _661, level(0.0)).xyz * fast::clamp(_678 * _677.z, 0.0, 1.0));
    }
    else
    {
        _693 = _673;
    }
    float3 _702 = (((((((((((((_129 * 0.5).xyz + (_176 * 0.5)).xyz + (_223 * 0.75)).xyz + (_270 * 0.5)).xyz + (_317 * 0.5)).xyz + (_364 * 0.75)).xyz + (_411 * 1.0)).xyz + (_458 * 0.75)).xyz + (_505 * 0.5)).xyz + (_552 * 0.5)).xyz + (_599 * 0.75)).xyz + (_646 * 0.5)).xyz + (_693 * 0.5)).xyz * float3(0.125);
    _28 _704 = _74;
    _704._m0 = float4(_702.x, _702.y, _702.z, float4(0.0).w);
    _28 _705 = _704;
    _705._m0.w = 1.0;
    out.m_5 = _705._m0;
    return out;
}

