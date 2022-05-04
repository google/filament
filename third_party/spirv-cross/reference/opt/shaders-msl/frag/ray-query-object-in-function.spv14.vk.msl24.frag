#include <metal_stdlib>
#include <simd/simd.h>
#if __METAL_VERSION__ >= 230
#include <metal_raytracing>
using namespace metal::raytracing;
#endif

using namespace metal;

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
    rayQuery.reset(ray(float3((in.inPos.xy * 4.0) - float2(2.0), 1.0), float3(0.0, 0.0, -1.0), 0.001000000047497451305389404296875, 2.0), topLevelAS, intersection_params());
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

