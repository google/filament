#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct world_info
{
    float4 ambient_light;
    float2 world_resolution;
    float2 level_size;
};

struct spvDescriptorSetBuffer1
{
    constant world_info* world_info [[id(0)]];
};

struct spvDescriptorSetBuffer2
{
    texture2d<float> ui [[id(0)]];
    sampler ui_sampler [[id(1)]];
    texture2d<float> scene [[id(2)]];
    sampler scene_sampler [[id(3)]];
};

struct main0_out
{
    float4 _entryPointOutput [[color(0)]];
};

struct main0_in
{
    float2 input_uv [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]], constant spvDescriptorSetBuffer1& spvDescriptorSet1 [[buffer(1)]], constant spvDescriptorSetBuffer2& spvDescriptorSet2 [[buffer(2)]], constant uint* spvDynamicOffsets [[buffer(23)]])
{
    constant auto& world_info_1 = *(constant world_info* )((constant char* )spvDescriptorSet1.world_info + spvDynamicOffsets[1]);
    main0_out out = {};
    float4 _176;
    do
    {
        float4 _134 = spvDescriptorSet2.ui.sample(spvDescriptorSet2.ui_sampler, in.input_uv);
        float4 _122 = _134;
        if (_122.w == 1.0)
        {
            _176 = _134;
            break;
        }
        float4 _124 = world_info_1.ambient_light;
        float _150 = _124.w;
        float4 _152 = world_info_1.ambient_light * _150;
        _124 = _152;
        _176 = _134 + (spvDescriptorSet2.scene.sample(spvDescriptorSet2.scene_sampler, in.input_uv) * _152);
        break;
    } while(false);
    out._entryPointOutput = _176;
    return out;
}

