#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct _15
{
    float4 _m0;
};

struct _3
{
    float4 _m0;
    float _m1;
    float4 _m2;
};

struct _4
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

struct _7
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

struct _8
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

struct _9
{
    float4 _m0;
};

constant _15 _10264 = {};

struct main0_out
{
    float4 m_4317 [[color(0)]];
};

fragment main0_out main0(constant _3& _22044 [[buffer(0)]], constant _4& _12348 [[buffer(1)]], constant _7& _15259 [[buffer(2)]], texture2d<float> _5785 [[texture(0)]], texture2d<float> _3312 [[texture(1)]], texture2d<float> _4862 [[texture(2)]], sampler _5688 [[sampler(0)]], sampler _4646 [[sampler(1)]], sampler _3594 [[sampler(2)]], float4 gl_FragCoord [[position]])
{
    main0_out out = {};
    _15 _13863;
    _13863._m0 = float4(0.0);
    float2 _19927 = gl_FragCoord.xy * _15259._m23.xy;
    float4 _17581 = _22044._m2 * _22044._m0.xyxy;
    float2 _13149 = fast::clamp(_19927 + (float3(0.0, -2.0, 0.5).xy * _22044._m0.xy), _17581.xy, _17581.zw);
    float3 _12103 = float3(_12348._m5) * fast::clamp(_5785.sample(_5688, _13149, level(0.0)).w * _22044._m1, 0.0, 1.0);
    float4 _17670 = _3312.sample(_4646, _13149, level(0.0));
    float3 _7719;
    if (_17670.y > 0.0)
    {
        _7719 = _12103 + (_4862.sample(_3594, _13149, level(0.0)).xyz * fast::clamp(_17670.y * _17670.z, 0.0, 1.0));
    }
    else
    {
        _7719 = _12103;
    }
    float3 _22177 = float4(0.0).xyz + (_7719 * 0.5);
    float4 _15527 = float4(_22177.x, _22177.y, _22177.z, float4(0.0).w);
    _13863._m0 = _15527;
    float2 _13150 = fast::clamp(_19927 + (float3(-1.0, -1.0, 0.5).xy * _22044._m0.xy), _17581.xy, _17581.zw);
    float3 _12104 = float3(_12348._m5) * fast::clamp(_5785.sample(_5688, _13150, level(0.0)).w * _22044._m1, 0.0, 1.0);
    float4 _17671 = _3312.sample(_4646, _13150, level(0.0));
    float3 _7720;
    if (_17671.y > 0.0)
    {
        _7720 = _12104 + (_4862.sample(_3594, _13150, level(0.0)).xyz * fast::clamp(_17671.y * _17671.z, 0.0, 1.0));
    }
    else
    {
        _7720 = _12104;
    }
    float3 _22178 = _15527.xyz + (_7720 * 0.5);
    float4 _15528 = float4(_22178.x, _22178.y, _22178.z, _15527.w);
    _13863._m0 = _15528;
    float2 _13151 = fast::clamp(_19927 + (float3(0.0, -1.0, 0.75).xy * _22044._m0.xy), _17581.xy, _17581.zw);
    float3 _12105 = float3(_12348._m5) * fast::clamp(_5785.sample(_5688, _13151, level(0.0)).w * _22044._m1, 0.0, 1.0);
    float4 _17672 = _3312.sample(_4646, _13151, level(0.0));
    float3 _7721;
    if (_17672.y > 0.0)
    {
        _7721 = _12105 + (_4862.sample(_3594, _13151, level(0.0)).xyz * fast::clamp(_17672.y * _17672.z, 0.0, 1.0));
    }
    else
    {
        _7721 = _12105;
    }
    float3 _22179 = _15528.xyz + (_7721 * 0.75);
    float4 _15529 = float4(_22179.x, _22179.y, _22179.z, _15528.w);
    _13863._m0 = _15529;
    float2 _13152 = fast::clamp(_19927 + (float3(1.0, -1.0, 0.5).xy * _22044._m0.xy), _17581.xy, _17581.zw);
    float3 _12106 = float3(_12348._m5) * fast::clamp(_5785.sample(_5688, _13152, level(0.0)).w * _22044._m1, 0.0, 1.0);
    float4 _17673 = _3312.sample(_4646, _13152, level(0.0));
    float3 _7722;
    if (_17673.y > 0.0)
    {
        _7722 = _12106 + (_4862.sample(_3594, _13152, level(0.0)).xyz * fast::clamp(_17673.y * _17673.z, 0.0, 1.0));
    }
    else
    {
        _7722 = _12106;
    }
    float3 _22180 = _15529.xyz + (_7722 * 0.5);
    float4 _15530 = float4(_22180.x, _22180.y, _22180.z, _15529.w);
    _13863._m0 = _15530;
    float2 _13153 = fast::clamp(_19927 + (float3(-2.0, 0.0, 0.5).xy * _22044._m0.xy), _17581.xy, _17581.zw);
    float3 _12107 = float3(_12348._m5) * fast::clamp(_5785.sample(_5688, _13153, level(0.0)).w * _22044._m1, 0.0, 1.0);
    float4 _17674 = _3312.sample(_4646, _13153, level(0.0));
    float3 _7723;
    if (_17674.y > 0.0)
    {
        _7723 = _12107 + (_4862.sample(_3594, _13153, level(0.0)).xyz * fast::clamp(_17674.y * _17674.z, 0.0, 1.0));
    }
    else
    {
        _7723 = _12107;
    }
    float3 _22181 = _15530.xyz + (_7723 * 0.5);
    float4 _15531 = float4(_22181.x, _22181.y, _22181.z, _15530.w);
    _13863._m0 = _15531;
    float2 _13154 = fast::clamp(_19927 + (float3(-1.0, 0.0, 0.75).xy * _22044._m0.xy), _17581.xy, _17581.zw);
    float3 _12108 = float3(_12348._m5) * fast::clamp(_5785.sample(_5688, _13154, level(0.0)).w * _22044._m1, 0.0, 1.0);
    float4 _17675 = _3312.sample(_4646, _13154, level(0.0));
    float3 _7724;
    if (_17675.y > 0.0)
    {
        _7724 = _12108 + (_4862.sample(_3594, _13154, level(0.0)).xyz * fast::clamp(_17675.y * _17675.z, 0.0, 1.0));
    }
    else
    {
        _7724 = _12108;
    }
    float3 _22182 = _15531.xyz + (_7724 * 0.75);
    float4 _15532 = float4(_22182.x, _22182.y, _22182.z, _15531.w);
    _13863._m0 = _15532;
    float2 _13155 = fast::clamp(_19927 + (float3(0.0, 0.0, 1.0).xy * _22044._m0.xy), _17581.xy, _17581.zw);
    float3 _12109 = float3(_12348._m5) * fast::clamp(_5785.sample(_5688, _13155, level(0.0)).w * _22044._m1, 0.0, 1.0);
    float4 _17676 = _3312.sample(_4646, _13155, level(0.0));
    float3 _7725;
    if (_17676.y > 0.0)
    {
        _7725 = _12109 + (_4862.sample(_3594, _13155, level(0.0)).xyz * fast::clamp(_17676.y * _17676.z, 0.0, 1.0));
    }
    else
    {
        _7725 = _12109;
    }
    float3 _22183 = _15532.xyz + (_7725 * 1.0);
    float4 _15533 = float4(_22183.x, _22183.y, _22183.z, _15532.w);
    _13863._m0 = _15533;
    float2 _13156 = fast::clamp(_19927 + (float3(1.0, 0.0, 0.75).xy * _22044._m0.xy), _17581.xy, _17581.zw);
    float3 _12110 = float3(_12348._m5) * fast::clamp(_5785.sample(_5688, _13156, level(0.0)).w * _22044._m1, 0.0, 1.0);
    float4 _17677 = _3312.sample(_4646, _13156, level(0.0));
    float3 _7726;
    if (_17677.y > 0.0)
    {
        _7726 = _12110 + (_4862.sample(_3594, _13156, level(0.0)).xyz * fast::clamp(_17677.y * _17677.z, 0.0, 1.0));
    }
    else
    {
        _7726 = _12110;
    }
    float3 _22184 = _15533.xyz + (_7726 * 0.75);
    float4 _15534 = float4(_22184.x, _22184.y, _22184.z, _15533.w);
    _13863._m0 = _15534;
    float2 _13157 = fast::clamp(_19927 + (float3(2.0, 0.0, 0.5).xy * _22044._m0.xy), _17581.xy, _17581.zw);
    float3 _12111 = float3(_12348._m5) * fast::clamp(_5785.sample(_5688, _13157, level(0.0)).w * _22044._m1, 0.0, 1.0);
    float4 _17678 = _3312.sample(_4646, _13157, level(0.0));
    float3 _7727;
    if (_17678.y > 0.0)
    {
        _7727 = _12111 + (_4862.sample(_3594, _13157, level(0.0)).xyz * fast::clamp(_17678.y * _17678.z, 0.0, 1.0));
    }
    else
    {
        _7727 = _12111;
    }
    float3 _22185 = _15534.xyz + (_7727 * 0.5);
    float4 _15535 = float4(_22185.x, _22185.y, _22185.z, _15534.w);
    _13863._m0 = _15535;
    float2 _13158 = fast::clamp(_19927 + (float3(-1.0, 1.0, 0.5).xy * _22044._m0.xy), _17581.xy, _17581.zw);
    float3 _12112 = float3(_12348._m5) * fast::clamp(_5785.sample(_5688, _13158, level(0.0)).w * _22044._m1, 0.0, 1.0);
    float4 _17679 = _3312.sample(_4646, _13158, level(0.0));
    float3 _7728;
    if (_17679.y > 0.0)
    {
        _7728 = _12112 + (_4862.sample(_3594, _13158, level(0.0)).xyz * fast::clamp(_17679.y * _17679.z, 0.0, 1.0));
    }
    else
    {
        _7728 = _12112;
    }
    float3 _22186 = _15535.xyz + (_7728 * 0.5);
    float4 _15536 = float4(_22186.x, _22186.y, _22186.z, _15535.w);
    _13863._m0 = _15536;
    float2 _13159 = fast::clamp(_19927 + (float3(0.0, 1.0, 0.75).xy * _22044._m0.xy), _17581.xy, _17581.zw);
    float3 _12113 = float3(_12348._m5) * fast::clamp(_5785.sample(_5688, _13159, level(0.0)).w * _22044._m1, 0.0, 1.0);
    float4 _17680 = _3312.sample(_4646, _13159, level(0.0));
    float3 _7729;
    if (_17680.y > 0.0)
    {
        _7729 = _12113 + (_4862.sample(_3594, _13159, level(0.0)).xyz * fast::clamp(_17680.y * _17680.z, 0.0, 1.0));
    }
    else
    {
        _7729 = _12113;
    }
    float3 _22187 = _15536.xyz + (_7729 * 0.75);
    float4 _15537 = float4(_22187.x, _22187.y, _22187.z, _15536.w);
    _13863._m0 = _15537;
    float2 _13160 = fast::clamp(_19927 + (float3(1.0, 1.0, 0.5).xy * _22044._m0.xy), _17581.xy, _17581.zw);
    float3 _12114 = float3(_12348._m5) * fast::clamp(_5785.sample(_5688, _13160, level(0.0)).w * _22044._m1, 0.0, 1.0);
    float4 _17681 = _3312.sample(_4646, _13160, level(0.0));
    float3 _7730;
    if (_17681.y > 0.0)
    {
        _7730 = _12114 + (_4862.sample(_3594, _13160, level(0.0)).xyz * fast::clamp(_17681.y * _17681.z, 0.0, 1.0));
    }
    else
    {
        _7730 = _12114;
    }
    float3 _22188 = _15537.xyz + (_7730 * 0.5);
    float4 _15539 = float4(_22188.x, _22188.y, _22188.z, _15537.w);
    _13863._m0 = _15539;
    float2 _13161 = fast::clamp(_19927 + (float3(0.0, 2.0, 0.5).xy * _22044._m0.xy), _17581.xy, _17581.zw);
    float3 _12115 = float3(_12348._m5) * fast::clamp(_5785.sample(_5688, _13161, level(0.0)).w * _22044._m1, 0.0, 1.0);
    float4 _17682 = _3312.sample(_4646, _13161, level(0.0));
    float3 _7731;
    if (_17682.y > 0.0)
    {
        _7731 = _12115 + (_4862.sample(_3594, _13161, level(0.0)).xyz * fast::clamp(_17682.y * _17682.z, 0.0, 1.0));
    }
    else
    {
        _7731 = _12115;
    }
    float3 _22189 = _15539.xyz + (_7731 * 0.5);
    float4 _15541 = float4(_22189.x, _22189.y, _22189.z, _15539.w);
    _13863._m0 = _15541;
    float3 _13750 = _15541.xyz / float3(((((((((((((0.0 + 0.5) + 0.5) + 0.75) + 0.5) + 0.5) + 0.75) + 1.0) + 0.75) + 0.5) + 0.5) + 0.75) + 0.5) + 0.5);
    _13863._m0 = float4(_13750.x, _13750.y, _13750.z, _15541.w);
    _13863._m0.w = 1.0;
    out.m_4317 = _13863._m0;
    return out;
}

