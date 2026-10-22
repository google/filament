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
    int _27 = min(ubo.spot_light_count, 4);
    float3 _79;
    _79 = float3(0.0);
    for (int _78 = 0; _78 < _27; )
    {
        _79 += ((float3(ubo.spot_lights[_78].data.color) * ubo.spot_lights[_78].data.intensity) * ubo.spot_lights[_78].data.penumbra);
        _78++;
        continue;
    }
    out.FragColor = float4(ubo.albedo[0u], ubo.roughness, ubo.alpha * _79.z, 1.0);
    return out;
}

