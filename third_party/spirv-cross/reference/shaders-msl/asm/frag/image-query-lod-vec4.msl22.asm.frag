#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float2 LOD [[color(0)]];
};

struct main0_in
{
    float4 Coord [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]], texture2d<float> Texture0 [[texture(0)]], texture2d_array<float> Texture1 [[texture(1)]], texture3d<float> Texture2 [[texture(2)]], texturecube<float> Texture3 [[texture(3)]], texturecube_array<float> Texture4 [[texture(4)]], sampler Texture0Smplr [[sampler(0)]], sampler Texture1Smplr [[sampler(1)]], sampler Texture2Smplr [[sampler(2)]], sampler Texture3Smplr [[sampler(3)]], sampler Texture4Smplr [[sampler(4)]])
{
    main0_out out = {};
    float2 _44;
    _44.x = Texture0.calculate_clamped_lod(Texture0Smplr, in.Coord.xy);
    _44.y = Texture0.calculate_unclamped_lod(Texture0Smplr, in.Coord.xy);
    float2 _45;
    _45.x = Texture1.calculate_clamped_lod(Texture1Smplr, in.Coord.xy);
    _45.y = Texture1.calculate_unclamped_lod(Texture1Smplr, in.Coord.xy);
    float2 _46;
    _46.x = Texture2.calculate_clamped_lod(Texture2Smplr, in.Coord.xyz);
    _46.y = Texture2.calculate_unclamped_lod(Texture2Smplr, in.Coord.xyz);
    float2 _47;
    _47.x = Texture3.calculate_clamped_lod(Texture3Smplr, in.Coord.xyz);
    _47.y = Texture3.calculate_unclamped_lod(Texture3Smplr, in.Coord.xyz);
    float2 _48;
    _48.x = Texture4.calculate_clamped_lod(Texture4Smplr, in.Coord.xyz);
    _48.y = Texture4.calculate_unclamped_lod(Texture4Smplr, in.Coord.xyz);
    out.LOD = ((_44 + _45) + (_46 + _47)) + _48;
    return out;
}

