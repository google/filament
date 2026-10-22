#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    uint2 FragColor [[color(0)]];
};

fragment main0_out main0(uint gl_SubgroupSize [[threads_per_simdgroup]], uint gl_SubgroupInvocationID [[thread_index_in_simdgroup]])
{
    main0_out out = {};
    out.FragColor.x = gl_SubgroupSize;
    out.FragColor.y = gl_SubgroupInvocationID;
    return out;
}

