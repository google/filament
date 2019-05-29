#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColors_0 [[color(0)]];
    float4 FragColors_1 [[color(1)]];
    float4 FragColor2 [[color(2)]];
    float4 FragColor3 [[color(3)]];
};

struct main0_in
{
    float3 vColor [[user(locn0)]];
};

void set_globals(thread float (&FragColors)[2], thread float3& vColor, thread float2& FragColor2, thread float3& FragColor3)
{
    FragColors[0] = vColor.x;
    FragColors[1] = vColor.y;
    FragColor2 = vColor.xz;
    FragColor3 = vColor.zzz;
}

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    float FragColors[2] = {};
    float2 FragColor2 = {};
    float3 FragColor3 = {};
    set_globals(FragColors, in.vColor, FragColor2, FragColor3);
    out.FragColors_0 = float4(FragColors[0]);
    out.FragColors_1 = float4(FragColors[1]);
    out.FragColor2 = FragColor2.xyyy;
    out.FragColor3 = FragColor3.xyzz;
    return out;
}

