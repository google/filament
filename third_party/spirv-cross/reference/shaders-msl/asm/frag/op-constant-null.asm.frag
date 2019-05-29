#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct D
{
    float4 a;
    float b;
};

constant float4 _14[4] = { float4(0.0), float4(0.0), float4(0.0), float4(0.0) };

struct main0_out
{
    float FragColor [[color(0)]];
};

fragment main0_out main0()
{
    main0_out out = {};
    float a = 0.0;
    float4 b = float4(0.0);
    float2x3 c = float2x3(float3(0.0), float3(0.0));
    D d = D{ float4(0.0), 0.0 };
    out.FragColor = a;
    return out;
}

