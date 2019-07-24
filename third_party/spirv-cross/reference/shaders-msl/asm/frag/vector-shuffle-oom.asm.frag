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

struct _20
{
    float4 _m0;
    float4 _m1;
    float2 _m2;
    float2 _m3;
    float3 _m4;
    float _m5;
    float3 _m6;
    float _m7;
    float4 _m8;
    float4 _m9;
    float4 _m10;
    float3 _m11;
    float _m12;
    float3 _m13;
    float _m14;
    float3 _m15;
    float4 _m16;
    float3 _m17;
    float _m18;
    float3 _m19;
    float2 _m20;
};

struct _21
{
    float4 _m0;
};

constant _28 _74 = {};

struct main0_out
{
    float4 m_5 [[color(0)]];
};

fragment main0_out main0(constant _6& _7 [[buffer(0)]], constant _10& _11 [[buffer(1)]], constant _18& _19 [[buffer(2)]], texture2d<float> _8 [[texture(0)]], texture2d<float> _12 [[texture(1)]], texture2d<float> _14 [[texture(2)]], sampler _9 [[sampler(0)]], sampler _13 [[sampler(1)]], sampler _15 [[sampler(2)]], float4 gl_FragCoord [[position]])
{
    main0_out out = {};
    _28 _77 = _74;
    _77._m0 = float4(0.0);
    float2 _82 = gl_FragCoord.xy * _19._m23.xy;
    float4 _88 = _7._m2 * _7._m0.xyxy;
    float2 _97 = fast::clamp(_82 + (float3(0.0, -2.0, 0.5).xy * _7._m0.xy), _88.xy, _88.zw);
    float3 _109 = float3(_11._m5) * fast::clamp(_8.sample(_9, _97, level(0.0)).w * _7._m1, 0.0, 1.0);
    float4 _113 = _12.sample(_13, _97, level(0.0));
    float3 _129;
    if (_113.y > 0.0)
    {
        _129 = _109 + (_14.sample(_15, _97, level(0.0)).xyz * fast::clamp(_113.y * _113.z, 0.0, 1.0));
    }
    else
    {
        _129 = _109;
    }
    float3 _133 = float4(0.0).xyz + (_129 * 0.5);
    float4 _134 = float4(_133.x, _133.y, _133.z, float4(0.0).w);
    _28 _135 = _77;
    _135._m0 = _134;
    float2 _144 = fast::clamp(_82 + (float3(-1.0, -1.0, 0.5).xy * _7._m0.xy), _88.xy, _88.zw);
    float3 _156 = float3(_11._m5) * fast::clamp(_8.sample(_9, _144, level(0.0)).w * _7._m1, 0.0, 1.0);
    float4 _160 = _12.sample(_13, _144, level(0.0));
    float3 _176;
    if (_160.y > 0.0)
    {
        _176 = _156 + (_14.sample(_15, _144, level(0.0)).xyz * fast::clamp(_160.y * _160.z, 0.0, 1.0));
    }
    else
    {
        _176 = _156;
    }
    float3 _180 = _134.xyz + (_176 * 0.5);
    float4 _181 = float4(_180.x, _180.y, _180.z, _134.w);
    _28 _182 = _135;
    _182._m0 = _181;
    float2 _191 = fast::clamp(_82 + (float3(0.0, -1.0, 0.75).xy * _7._m0.xy), _88.xy, _88.zw);
    float3 _203 = float3(_11._m5) * fast::clamp(_8.sample(_9, _191, level(0.0)).w * _7._m1, 0.0, 1.0);
    float4 _207 = _12.sample(_13, _191, level(0.0));
    float3 _223;
    if (_207.y > 0.0)
    {
        _223 = _203 + (_14.sample(_15, _191, level(0.0)).xyz * fast::clamp(_207.y * _207.z, 0.0, 1.0));
    }
    else
    {
        _223 = _203;
    }
    float3 _227 = _181.xyz + (_223 * 0.75);
    float4 _228 = float4(_227.x, _227.y, _227.z, _181.w);
    _28 _229 = _182;
    _229._m0 = _228;
    float2 _238 = fast::clamp(_82 + (float3(1.0, -1.0, 0.5).xy * _7._m0.xy), _88.xy, _88.zw);
    float3 _250 = float3(_11._m5) * fast::clamp(_8.sample(_9, _238, level(0.0)).w * _7._m1, 0.0, 1.0);
    float4 _254 = _12.sample(_13, _238, level(0.0));
    float3 _270;
    if (_254.y > 0.0)
    {
        _270 = _250 + (_14.sample(_15, _238, level(0.0)).xyz * fast::clamp(_254.y * _254.z, 0.0, 1.0));
    }
    else
    {
        _270 = _250;
    }
    float3 _274 = _228.xyz + (_270 * 0.5);
    float4 _275 = float4(_274.x, _274.y, _274.z, _228.w);
    _28 _276 = _229;
    _276._m0 = _275;
    float2 _285 = fast::clamp(_82 + (float3(-2.0, 0.0, 0.5).xy * _7._m0.xy), _88.xy, _88.zw);
    float3 _297 = float3(_11._m5) * fast::clamp(_8.sample(_9, _285, level(0.0)).w * _7._m1, 0.0, 1.0);
    float4 _301 = _12.sample(_13, _285, level(0.0));
    float3 _317;
    if (_301.y > 0.0)
    {
        _317 = _297 + (_14.sample(_15, _285, level(0.0)).xyz * fast::clamp(_301.y * _301.z, 0.0, 1.0));
    }
    else
    {
        _317 = _297;
    }
    float3 _321 = _275.xyz + (_317 * 0.5);
    float4 _322 = float4(_321.x, _321.y, _321.z, _275.w);
    _28 _323 = _276;
    _323._m0 = _322;
    float2 _332 = fast::clamp(_82 + (float3(-1.0, 0.0, 0.75).xy * _7._m0.xy), _88.xy, _88.zw);
    float3 _344 = float3(_11._m5) * fast::clamp(_8.sample(_9, _332, level(0.0)).w * _7._m1, 0.0, 1.0);
    float4 _348 = _12.sample(_13, _332, level(0.0));
    float3 _364;
    if (_348.y > 0.0)
    {
        _364 = _344 + (_14.sample(_15, _332, level(0.0)).xyz * fast::clamp(_348.y * _348.z, 0.0, 1.0));
    }
    else
    {
        _364 = _344;
    }
    float3 _368 = _322.xyz + (_364 * 0.75);
    float4 _369 = float4(_368.x, _368.y, _368.z, _322.w);
    _28 _370 = _323;
    _370._m0 = _369;
    float2 _379 = fast::clamp(_82 + (float3(0.0, 0.0, 1.0).xy * _7._m0.xy), _88.xy, _88.zw);
    float3 _391 = float3(_11._m5) * fast::clamp(_8.sample(_9, _379, level(0.0)).w * _7._m1, 0.0, 1.0);
    float4 _395 = _12.sample(_13, _379, level(0.0));
    float3 _411;
    if (_395.y > 0.0)
    {
        _411 = _391 + (_14.sample(_15, _379, level(0.0)).xyz * fast::clamp(_395.y * _395.z, 0.0, 1.0));
    }
    else
    {
        _411 = _391;
    }
    float3 _415 = _369.xyz + (_411 * 1.0);
    float4 _416 = float4(_415.x, _415.y, _415.z, _369.w);
    _28 _417 = _370;
    _417._m0 = _416;
    float2 _426 = fast::clamp(_82 + (float3(1.0, 0.0, 0.75).xy * _7._m0.xy), _88.xy, _88.zw);
    float3 _438 = float3(_11._m5) * fast::clamp(_8.sample(_9, _426, level(0.0)).w * _7._m1, 0.0, 1.0);
    float4 _442 = _12.sample(_13, _426, level(0.0));
    float3 _458;
    if (_442.y > 0.0)
    {
        _458 = _438 + (_14.sample(_15, _426, level(0.0)).xyz * fast::clamp(_442.y * _442.z, 0.0, 1.0));
    }
    else
    {
        _458 = _438;
    }
    float3 _462 = _416.xyz + (_458 * 0.75);
    float4 _463 = float4(_462.x, _462.y, _462.z, _416.w);
    _28 _464 = _417;
    _464._m0 = _463;
    float2 _473 = fast::clamp(_82 + (float3(2.0, 0.0, 0.5).xy * _7._m0.xy), _88.xy, _88.zw);
    float3 _485 = float3(_11._m5) * fast::clamp(_8.sample(_9, _473, level(0.0)).w * _7._m1, 0.0, 1.0);
    float4 _489 = _12.sample(_13, _473, level(0.0));
    float3 _505;
    if (_489.y > 0.0)
    {
        _505 = _485 + (_14.sample(_15, _473, level(0.0)).xyz * fast::clamp(_489.y * _489.z, 0.0, 1.0));
    }
    else
    {
        _505 = _485;
    }
    float3 _509 = _463.xyz + (_505 * 0.5);
    float4 _510 = float4(_509.x, _509.y, _509.z, _463.w);
    _28 _511 = _464;
    _511._m0 = _510;
    float2 _520 = fast::clamp(_82 + (float3(-1.0, 1.0, 0.5).xy * _7._m0.xy), _88.xy, _88.zw);
    float3 _532 = float3(_11._m5) * fast::clamp(_8.sample(_9, _520, level(0.0)).w * _7._m1, 0.0, 1.0);
    float4 _536 = _12.sample(_13, _520, level(0.0));
    float3 _552;
    if (_536.y > 0.0)
    {
        _552 = _532 + (_14.sample(_15, _520, level(0.0)).xyz * fast::clamp(_536.y * _536.z, 0.0, 1.0));
    }
    else
    {
        _552 = _532;
    }
    float3 _556 = _510.xyz + (_552 * 0.5);
    float4 _557 = float4(_556.x, _556.y, _556.z, _510.w);
    _28 _558 = _511;
    _558._m0 = _557;
    float2 _567 = fast::clamp(_82 + (float3(0.0, 1.0, 0.75).xy * _7._m0.xy), _88.xy, _88.zw);
    float3 _579 = float3(_11._m5) * fast::clamp(_8.sample(_9, _567, level(0.0)).w * _7._m1, 0.0, 1.0);
    float4 _583 = _12.sample(_13, _567, level(0.0));
    float3 _599;
    if (_583.y > 0.0)
    {
        _599 = _579 + (_14.sample(_15, _567, level(0.0)).xyz * fast::clamp(_583.y * _583.z, 0.0, 1.0));
    }
    else
    {
        _599 = _579;
    }
    float3 _603 = _557.xyz + (_599 * 0.75);
    float4 _604 = float4(_603.x, _603.y, _603.z, _557.w);
    _28 _605 = _558;
    _605._m0 = _604;
    float2 _614 = fast::clamp(_82 + (float3(1.0, 1.0, 0.5).xy * _7._m0.xy), _88.xy, _88.zw);
    float3 _626 = float3(_11._m5) * fast::clamp(_8.sample(_9, _614, level(0.0)).w * _7._m1, 0.0, 1.0);
    float4 _630 = _12.sample(_13, _614, level(0.0));
    float3 _646;
    if (_630.y > 0.0)
    {
        _646 = _626 + (_14.sample(_15, _614, level(0.0)).xyz * fast::clamp(_630.y * _630.z, 0.0, 1.0));
    }
    else
    {
        _646 = _626;
    }
    float3 _650 = _604.xyz + (_646 * 0.5);
    float4 _651 = float4(_650.x, _650.y, _650.z, _604.w);
    _28 _652 = _605;
    _652._m0 = _651;
    float2 _661 = fast::clamp(_82 + (float3(0.0, 2.0, 0.5).xy * _7._m0.xy), _88.xy, _88.zw);
    float3 _673 = float3(_11._m5) * fast::clamp(_8.sample(_9, _661, level(0.0)).w * _7._m1, 0.0, 1.0);
    float4 _677 = _12.sample(_13, _661, level(0.0));
    float3 _693;
    if (_677.y > 0.0)
    {
        _693 = _673 + (_14.sample(_15, _661, level(0.0)).xyz * fast::clamp(_677.y * _677.z, 0.0, 1.0));
    }
    else
    {
        _693 = _673;
    }
    float3 _697 = _651.xyz + (_693 * 0.5);
    float4 _698 = float4(_697.x, _697.y, _697.z, _651.w);
    _28 _699 = _652;
    _699._m0 = _698;
    float3 _702 = _698.xyz / float3(((((((((((((0.0 + 0.5) + 0.5) + 0.75) + 0.5) + 0.5) + 0.75) + 1.0) + 0.75) + 0.5) + 0.5) + 0.75) + 0.5) + 0.5);
    _28 _704 = _699;
    _704._m0 = float4(_702.x, _702.y, _702.z, _698.w);
    _28 _705 = _704;
    _705._m0.w = 1.0;
    out.m_5 = _705._m0;
    return out;
}

