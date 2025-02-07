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

static inline __attribute__((always_inline))
void initFn(thread raytracing::intersection_query<raytracing::instancing, raytracing::triangle_data>& rayQuery, const raytracing::acceleration_structure<raytracing::instancing> topLevelAS)
{
    float3 rayOrigin = float3(0.0, 0.0, 1.0);
    float3 rayDirection = float3(0.0, 0.0, -1.0);
    float rayDistance = 2.0;
    rayQuery.reset(ray(rayOrigin, rayDirection, 0.001000000047497451305389404296875, rayDistance), topLevelAS, 255u, spvMakeIntersectionParams(4u));
}

static inline __attribute__((always_inline))
uint proceeFn(thread raytracing::intersection_query<raytracing::instancing, raytracing::triangle_data>& rayQuery)
{
    for (;;)
    {
        bool _46 = rayQuery.next();
        if (_46)
        {
            continue;
        }
        else
        {
            break;
        }
    }
    uint _50 = uint(rayQuery.get_committed_intersection_type());
    return _50;
}

fragment void main0(raytracing::acceleration_structure<raytracing::instancing> topLevelAS [[buffer(0)]])
{
    raytracing::intersection_query<raytracing::instancing, raytracing::triangle_data> rayQuery;
    initFn(rayQuery, topLevelAS);
    uint _55 = proceeFn(rayQuery);
}

