#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor0 [[color(0)]];
};

fragment main0_out main0()
{
    main0_out out = {};
    float FragColor0 = {};
    float2 FragColor1 = {};
    float FragColor3 = {};
    FragColor0 = 1.0;
    FragColor1 = float2(2.0, 3.0);
    FragColor3 = 4.0;
    out.FragColor0.x = FragColor0;
    out.FragColor0.yz = FragColor1;
    out.FragColor0.w = FragColor3;
    return out;
}

