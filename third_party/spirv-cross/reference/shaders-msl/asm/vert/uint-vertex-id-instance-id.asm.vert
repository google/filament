#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 gl_Position [[position]];
};

static inline __attribute__((always_inline))
float4 _main(thread const uint& vid, thread const uint& iid)
{
    return float4(float(vid + iid));
}

vertex main0_out main0(uint gl_VertexIndex [[vertex_id]], uint gl_InstanceIndex [[instance_id]])
{
    main0_out out = {};
    uint vid = gl_VertexIndex;
    uint iid = gl_InstanceIndex;
    uint param = vid;
    uint param_1 = iid;
    out.gl_Position = _main(param, param_1);
    return out;
}

