#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wmissing-braces"
#pragma clang diagnostic ignored "-Wunused-variable"

#include <metal_stdlib>
#include <simd/simd.h>
#include <metal_atomic>

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

struct type_StructuredBuffer_v4float
{
    float4 _m0[1];
};

struct type_Globals
{
    uint2 ShadowTileListGroupSize;
};

constant float3 _70 = {};

struct main0_out
{
    float4 out_var_SV_Target0 [[color(0)]];
};

struct main0_in
{
    uint in_var_TEXCOORD0 [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]], const device type_StructuredBuffer_v4float& CulledObjectBoxBounds [[buffer(0)]], constant type_Globals& _Globals [[buffer(1)]], texture2d<uint> RWShadowTileNumCulledObjects [[texture(0)]], device atomic_uint* RWShadowTileNumCulledObjects_atomic [[buffer(2)]], float4 gl_FragCoord [[position]])
{
    main0_out out = {};
    uint2 _77 = uint2(gl_FragCoord.xy);
    uint _78 = _77.y;
    uint _83 = _77.x;
    float2 _91 = float2(float(_83), float((_Globals.ShadowTileListGroupSize.y - 1u) - _78));
    float2 _93 = float2(_Globals.ShadowTileListGroupSize);
    float2 _96 = ((_91 / _93) * float2(2.0)) - float2(1.0);
    float2 _100 = (((_91 + float2(1.0)) / _93) * float2(2.0)) - float2(1.0);
    float3 _102 = float3(_100.x, _100.y, _70.z);
    _102.z = 1.0;
    uint _103 = in.in_var_TEXCOORD0 * 5u;
    uint _107 = _103 + 1u;
    if (all(CulledObjectBoxBounds._m0[_107].xy > _96.xy) && all(CulledObjectBoxBounds._m0[_103].xyz < _102))
    {
        float3 _121 = float3(0.5) * (CulledObjectBoxBounds._m0[_103].xyz + CulledObjectBoxBounds._m0[_107].xyz);
        float _122 = _96.x;
        float _123 = _96.y;
        spvUnsafeArray<float3, 8> _73;
        _73[0] = float3(_122, _123, -1000.0);
        float _126 = _100.x;
        _73[1] = float3(_126, _123, -1000.0);
        float _129 = _100.y;
        _73[2] = float3(_122, _129, -1000.0);
        _73[3] = float3(_126, _129, -1000.0);
        _73[4] = float3(_122, _123, 1.0);
        _73[5] = float3(_126, _123, 1.0);
        _73[6] = float3(_122, _129, 1.0);
        _73[7] = float3(_126, _129, 1.0);
        float3 _155;
        float3 _158;
        _155 = float3(-500000.0);
        _158 = float3(500000.0);
        for (int _160 = 0; _160 < 8; )
        {
            float3 _166 = _73[_160] - _121;
            float3 _170 = float3(dot(_166, CulledObjectBoxBounds._m0[_103 + 2u].xyz), dot(_166, CulledObjectBoxBounds._m0[_103 + 3u].xyz), dot(_166, CulledObjectBoxBounds._m0[_103 + 4u].xyz));
            _155 = fast::max(_155, _170);
            _158 = fast::min(_158, _170);
            _160++;
            continue;
        }
        if (all(_158 < float3(1.0)) && all(_155 > float3(-1.0)))
        {
            uint _179 = atomic_fetch_add_explicit((device atomic_uint*)&RWShadowTileNumCulledObjects_atomic[(_78 * _Globals.ShadowTileListGroupSize.x) + _83], 1u, memory_order_relaxed);
        }
    }
    out.out_var_SV_Target0 = float4(0.0);
    return out;
}

