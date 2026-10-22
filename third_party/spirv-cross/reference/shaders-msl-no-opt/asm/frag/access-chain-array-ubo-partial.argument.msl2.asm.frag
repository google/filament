#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct UBOs
{
    float4 v;
};

struct spvDescriptorSetBuffer0
{
    constant UBOs* ubos [[id(0)]][2];
};

struct main0_out
{
    float4 FragColor [[color(0)]];
};

fragment main0_out main0(constant spvDescriptorSetBuffer0& spvDescriptorSet0 [[buffer(0)]])
{
    main0_out out = {};
    out.FragColor = spvDescriptorSet0.ubos[0]->v + spvDescriptorSet0.ubos[1]->v;
    return out;
}

