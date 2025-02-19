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

static inline __attribute__((always_inline))
uint doRay(thread const float3& rayOrigin, thread const float3& rayDirection, thread const float& rayDistance, thread raytracing::intersection_query<raytracing::instancing, raytracing::triangle_data>& rayQuery, const raytracing::acceleration_structure<raytracing::instancing> topLevelAS)
{
    rayQuery.reset(ray(rayOrigin, rayDirection, 0.001000000047497451305389404296875, rayDistance), topLevelAS, 255u, spvMakeIntersectionParams(4u));
    for (;;)
    {
        bool _36 = rayQuery.next();
        if (_36)
        {
            continue;
        }
        else
        {
            break;
        }
    }
    uint _40 = uint(rayQuery.get_committed_intersection_type());
    return _40;
}

fragment main0_out main0(main0_in in [[stage_in]], raytracing::acceleration_structure<raytracing::instancing> topLevelAS [[buffer(0)]])
{
    main0_out out = {};
    float3 rayOrigin = float3((in.inPos.xy * 4.0) - float2(2.0), 1.0);
    float3 rayDirection = float3(0.0, 0.0, -1.0);
    float rayDistance = 2.0;
    float3 param = rayOrigin;
    float3 param_1 = rayDirection;
    float param_2 = rayDistance;
    raytracing::intersection_query<raytracing::instancing, raytracing::triangle_data> rayQuery;
    uint _70 = doRay(param, param_1, param_2, rayQuery, topLevelAS);
    if (_70 == 0u)
    {
        discard_fragment();
    }
    out.outColor = in.inPos;
    return out;
}

