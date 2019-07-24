#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 outFragColor [[color(0)]];
};

struct main0_in
{
    float3 inPos [[user(locn0)]];
    float3 inNormal [[user(locn1)]];
    float4 inInvModelView_0 [[user(locn2)]];
    float4 inInvModelView_1 [[user(locn3)]];
    float4 inInvModelView_2 [[user(locn4)]];
    float4 inInvModelView_3 [[user(locn5)]];
    float inLodBias [[user(locn6)]];
};

fragment main0_out main0(main0_in in [[stage_in]], texturecube<float> samplerColor [[texture(0)]], sampler samplerColorSmplr [[sampler(0)]])
{
    main0_out out = {};
    float4x4 inInvModelView = {};
    inInvModelView[0] = in.inInvModelView_0;
    inInvModelView[1] = in.inInvModelView_1;
    inInvModelView[2] = in.inInvModelView_2;
    inInvModelView[3] = in.inInvModelView_3;
    float4 _31 = inInvModelView * float4(reflect(normalize(in.inPos), normalize(in.inNormal)), 0.0);
    float _33 = _31.x;
    float3 _59 = float3(_33, _31.yz);
    _59.x = _33 * (-1.0);
    out.outFragColor = samplerColor.sample(samplerColorSmplr, _59, bias(in.inLodBias));
    return out;
}

