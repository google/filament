#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>
#if __METAL_VERSION__ >= 230
#include <metal_raytracing>
using namespace metal::raytracing;
#endif

using namespace metal;

intersection_params spvMakeIntersectionParams(uint flags)
{
    intersection_params ip;
    if ((flags & 1) != 0)
        ip.force_opacity(forced_opacity::opaque);
    if ((flags & 2) != 0)
        ip.force_opacity(forced_opacity::non_opaque);
    if ((flags & 4) != 0)
        ip.accept_any_intersection(true);
    if ((flags & 16) != 0)
        ip.set_triangle_cull_mode(triangle_cull_mode::back);
    if ((flags & 32) != 0)
        ip.set_triangle_cull_mode(triangle_cull_mode::front);
    if ((flags & 64) != 0)
        ip.set_opacity_cull_mode(opacity_cull_mode::opaque);
    if ((flags & 128) != 0)
        ip.set_opacity_cull_mode(opacity_cull_mode::non_opaque);
    if ((flags & 256) != 0)
        ip.set_geometry_cull_mode(geometry_cull_mode::triangle);
    if ((flags & 512) != 0)
        ip.set_geometry_cull_mode(geometry_cull_mode::bounding_box);
    return ip;
}

struct main0_out
{
    float4 outColor [[color(0)]];
};

struct main0_in
{
    float4 inPos [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]], raytracing::acceleration_structure<raytracing::instancing> topLevelAS [[buffer(0)]])
{
    main0_out out = {};
    raytracing::intersection_query<raytracing::instancing, raytracing::triangle_data> rayQuery;
    rayQuery.reset(ray(float3((in.inPos.xy * 4.0) - float2(2.0), 1.0), float3(0.0, 0.0, -1.0), 0.001000000047497451305389404296875, 2.0), topLevelAS, 255u, spvMakeIntersectionParams(4u));
    for (;;)
    {
        bool _88 = rayQuery.next();
        if (_88)
        {
            continue;
        }
        else
        {
            break;
        }
    }
    uint _92 = uint(rayQuery.get_committed_intersection_type());
    if (_92 == 0u)
    {
        discard_fragment();
    }
    out.outColor = in.inPos;
    return out;
}

