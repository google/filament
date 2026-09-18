#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

template <typename T, int stride>
struct spvPaddedArrayElement { T data; char padding[stride - sizeof(T)]; };

struct SpotLight
{
    packed_float3 position;
    float range;
    packed_float3 direction;
    float angle;
    packed_float3 color;
    float intensity;
    float penumbra;
};

struct UBO
{
    spvPaddedArrayElement<SpotLight, 64> spot_lights[4];
    int spot_light_count;
    char _m2_pad[12];
    packed_float3 albedo;
    float roughness;
    float alpha;
};

struct main0_out
{
    float4 FragColor [[color(0)]];
};

fragment main0_out main0(constant UBO& ubo [[buffer(0)]])
{
    main0_out out = {};
    float3 acc = float3(0.0);
    int n = min(ubo.spot_light_count, 4);
    for (int i = 0; i < n; i++)
    {
        acc += ((float3(ubo.spot_lights[i].data.color) * ubo.spot_lights[i].data.intensity) * ubo.spot_lights[i].data.penumbra);
    }
    out.FragColor = float4(ubo.albedo[0u], ubo.roughness, ubo.alpha * acc.z, 1.0);
    return out;
}

