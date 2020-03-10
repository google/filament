#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wmissing-braces"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

template<typename T, size_t Num>
struct spvUnsafeArray
{
    T elements[Num ? Num : 1];
    
    thread T& operator [] (size_t pos) thread
    {
        return elements[pos];
    }
    constexpr const thread T& operator [] (size_t pos) const thread
    {
        return elements[pos];
    }
    
    device T& operator [] (size_t pos) device
    {
        return elements[pos];
    }
    constexpr const device T& operator [] (size_t pos) const device
    {
        return elements[pos];
    }
    
    constexpr const constant T& operator [] (size_t pos) const constant
    {
        return elements[pos];
    }
    
    threadgroup T& operator [] (size_t pos) threadgroup
    {
        return elements[pos];
    }
    constexpr const threadgroup T& operator [] (size_t pos) const threadgroup
    {
        return elements[pos];
    }
};

struct type_Globals
{
    float4 ViewportSize;
    float ScatteringScaling;
    float CocRadiusToCircumscribedRadius;
};

struct type_StructuredBuffer_v4float
{
    float4 _m0[1];
};

struct main0_out
{
    float2 out_var_TEXCOORD0 [[user(locn0)]];
    float4 out_var_TEXCOORD1 [[user(locn1)]];
    float4 out_var_TEXCOORD2 [[user(locn2)]];
    float4 out_var_TEXCOORD3 [[user(locn3)]];
    float4 out_var_TEXCOORD4 [[user(locn4)]];
    float4 out_var_TEXCOORD5 [[user(locn5)]];
    float4 out_var_TEXCOORD6 [[user(locn6)]];
    float4 gl_Position [[position]];
};

vertex main0_out main0(constant type_Globals& _Globals [[buffer(0)]], const device type_StructuredBuffer_v4float& ScatterDrawList [[buffer(1)]], uint gl_VertexIndex [[vertex_id]], uint gl_InstanceIndex [[instance_id]])
{
    main0_out out = {};
    uint _66 = gl_VertexIndex / 4u;
    uint _68 = gl_VertexIndex - (_66 * 4u);
    uint _70 = (16u * gl_InstanceIndex) + _66;
    float _72;
    _72 = 0.0;
    spvUnsafeArray<float4, 4> _61;
    spvUnsafeArray<float, 4> _62;
    spvUnsafeArray<float2, 4> _63;
    float _73;
    uint _75 = 0u;
    for (;;)
    {
        if (_75 < 4u)
        {
            uint _82 = ((5u * _70) + _75) + 1u;
            _61[_75] = float4(ScatterDrawList._m0[_82].xyz, 0.0);
            _62[_75] = ScatterDrawList._m0[_82].w;
            if (_75 == 0u)
            {
                _73 = _62[_75];
            }
            else
            {
                _73 = fast::max(_72, _62[_75]);
            }
            _63[_75].x = (-0.5) / _62[_75];
            _63[_75].y = (0.5 * _62[_75]) + 0.5;
            _72 = _73;
            _75++;
            continue;
        }
        else
        {
            break;
        }
    }
    float2 _144 = float2(_Globals.ScatteringScaling) * ScatterDrawList._m0[5u * _70].xy;
    float2 _173 = (((float2((_72 * _Globals.CocRadiusToCircumscribedRadius) + 1.0) * ((float2(float(_68 % 2u), float(_68 / 2u)) * float2(2.0)) - float2(1.0))) + _144) + float2(0.5)) * _Globals.ViewportSize.zw;
    out.out_var_TEXCOORD0 = _144;
    out.out_var_TEXCOORD1 = float4(_61[0].xyz, _62[0]);
    out.out_var_TEXCOORD2 = float4(_61[1].xyz, _62[1]);
    out.out_var_TEXCOORD3 = float4(_61[2].xyz, _62[2]);
    out.out_var_TEXCOORD4 = float4(_61[3].xyz, _62[3]);
    out.out_var_TEXCOORD5 = float4(_63[0].x, _63[0].y, _63[1].x, _63[1].y);
    out.out_var_TEXCOORD6 = float4(_63[2].x, _63[2].y, _63[3].x, _63[3].y);
    out.gl_Position = float4((_173.x * 2.0) - 1.0, 1.0 - (_173.y * 2.0), 0.0, 1.0);
    return out;
}

