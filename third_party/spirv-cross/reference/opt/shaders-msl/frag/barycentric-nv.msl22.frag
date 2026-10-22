#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct Vertices
{
    float2 uvs[1];
};

struct main0_out
{
    float2 value [[color(0)]];
};

struct main0_in
{
    float3 gl_BaryCoordEXT [[barycentric_coord, center_perspective]];
};

fragment main0_out main0(main0_in in [[stage_in]], const device Vertices& _19 [[buffer(0)]], uint gl_PrimitiveID [[primitive_id]])
{
    main0_out out = {};
    int _23 = 3 * int(gl_PrimitiveID);
    out.value = ((_19.uvs[_23] * in.gl_BaryCoordEXT.x) + (_19.uvs[_23 + 1] * in.gl_BaryCoordEXT.y)) + (_19.uvs[_23 + 2] * in.gl_BaryCoordEXT.z);
    return out;
}

