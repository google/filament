#pragma clang diagnostic ignored "-Wunused-variable"

#include <metal_stdlib>
#include <simd/simd.h>
#include <metal_atomic>

using namespace metal;

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
    float2 _96 = fma(_91 / _93, float2(2.0), float2(-1.0));
    float2 _100 = fma((_91 + float2(1.0)) / _93, float2(2.0), float2(-1.0));
    float3 _101 = float3(_100.x, _100.y, _70.z);
    _101.z = 1.0;
    uint _103 = in.in_var_TEXCOORD0 * 5u;
    uint _107 = _103 + 1u;
    if (all(CulledObjectBoxBounds._m0[_107].xy > _96.xy) && all(CulledObjectBoxBounds._m0[_103].xyz < _101))
    {
        float3 _120 = CulledObjectBoxBounds._m0[_103].xyz + CulledObjectBoxBounds._m0[_107].xyz;
        float _122 = _96.x;
        float _123 = _96.y;
        float _126 = _100.x;
        float _129 = _100.y;
        float3 _166 = fma(float3(-0.5), _120, float3(_122, _123, -1000.0));
        float3 _170 = float3(dot(_166, CulledObjectBoxBounds._m0[_103 + 2u].xyz), dot(_166, CulledObjectBoxBounds._m0[_103 + 3u].xyz), dot(_166, CulledObjectBoxBounds._m0[_103 + 4u].xyz));
        float3 _189 = fma(float3(-0.5), _120, float3(_126, _123, -1000.0));
        float3 _193 = float3(dot(_189, CulledObjectBoxBounds._m0[_103 + 2u].xyz), dot(_189, CulledObjectBoxBounds._m0[_103 + 3u].xyz), dot(_189, CulledObjectBoxBounds._m0[_103 + 4u].xyz));
        float3 _205 = fma(float3(-0.5), _120, float3(_122, _129, -1000.0));
        float3 _209 = float3(dot(_205, CulledObjectBoxBounds._m0[_103 + 2u].xyz), dot(_205, CulledObjectBoxBounds._m0[_103 + 3u].xyz), dot(_205, CulledObjectBoxBounds._m0[_103 + 4u].xyz));
        float3 _221 = fma(float3(-0.5), _120, float3(_126, _129, -1000.0));
        float3 _225 = float3(dot(_221, CulledObjectBoxBounds._m0[_103 + 2u].xyz), dot(_221, CulledObjectBoxBounds._m0[_103 + 3u].xyz), dot(_221, CulledObjectBoxBounds._m0[_103 + 4u].xyz));
        float3 _237 = fma(float3(-0.5), _120, float3(_122, _123, 1.0));
        float3 _241 = float3(dot(_237, CulledObjectBoxBounds._m0[_103 + 2u].xyz), dot(_237, CulledObjectBoxBounds._m0[_103 + 3u].xyz), dot(_237, CulledObjectBoxBounds._m0[_103 + 4u].xyz));
        float3 _253 = fma(float3(-0.5), _120, float3(_126, _123, 1.0));
        float3 _257 = float3(dot(_253, CulledObjectBoxBounds._m0[_103 + 2u].xyz), dot(_253, CulledObjectBoxBounds._m0[_103 + 3u].xyz), dot(_253, CulledObjectBoxBounds._m0[_103 + 4u].xyz));
        float3 _269 = fma(float3(-0.5), _120, float3(_122, _129, 1.0));
        float3 _273 = float3(dot(_269, CulledObjectBoxBounds._m0[_103 + 2u].xyz), dot(_269, CulledObjectBoxBounds._m0[_103 + 3u].xyz), dot(_269, CulledObjectBoxBounds._m0[_103 + 4u].xyz));
        float3 _285 = fma(float3(-0.5), _120, float3(_126, _129, 1.0));
        float3 _289 = float3(dot(_285, CulledObjectBoxBounds._m0[_103 + 2u].xyz), dot(_285, CulledObjectBoxBounds._m0[_103 + 3u].xyz), dot(_285, CulledObjectBoxBounds._m0[_103 + 4u].xyz));
        if (all(fast::min(fast::min(fast::min(fast::min(fast::min(fast::min(fast::min(fast::min(float3(500000.0), _170), _193), _209), _225), _241), _257), _273), _289) < float3(1.0)) && all(fast::max(fast::max(fast::max(fast::max(fast::max(fast::max(fast::max(fast::max(float3(-500000.0), _170), _193), _209), _225), _241), _257), _273), _289) > float3(-1.0)))
        {
            uint _179 = atomic_fetch_add_explicit((device atomic_uint*)&RWShadowTileNumCulledObjects_atomic[(_78 * _Globals.ShadowTileListGroupSize.x) + _83], 1u, memory_order_relaxed);
        }
    }
    out.out_var_SV_Target0 = float4(0.0);
    return out;
}

