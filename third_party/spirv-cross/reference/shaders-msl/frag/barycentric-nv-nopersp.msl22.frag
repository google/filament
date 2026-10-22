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
    float3 gl_BaryCoordNoPerspEXT [[barycentric_coord, center_no_perspective]];
};

fragment main0_out main0(main0_in in [[stage_in]], const device Vertices& _19 [[buffer(0)]], uint gl_PrimitiveID [[primitive_id]])
{
    main0_out out = {};
    int prim = int(gl_PrimitiveID);
    float2 uv0 = _19.uvs[(3 * prim) + 0];
    float2 uv1 = _19.uvs[(3 * prim) + 1];
    float2 uv2 = _19.uvs[(3 * prim) + 2];
    out.value = ((uv0 * in.gl_BaryCoordNoPerspEXT.x) + (uv1 * in.gl_BaryCoordNoPerspEXT.y)) + (uv2 * in.gl_BaryCoordNoPerspEXT.z);
    return out;
}

