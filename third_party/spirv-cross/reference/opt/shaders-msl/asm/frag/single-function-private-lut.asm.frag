#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct myType
{
    float data;
};

constant myType _21[5] = { myType{ 0.0 }, myType{ 1.0 }, myType{ 0.0 }, myType{ 1.0 }, myType{ 0.0 } };

struct main0_out
{
    float4 o_color [[color(0)]];
};

// Implementation of the GLSL mod() function, which is slightly different than Metal fmod()
template<typename Tx, typename Ty>
Tx mod(Tx x, Ty y)
{
    return x - y * floor(x / y);
}

fragment main0_out main0(float4 gl_FragCoord [[position]])
{
    main0_out out = {};
    if (_21[int(mod(gl_FragCoord.x, 4.0))].data > 0.0)
    {
        out.o_color = float4(0.0, 1.0, 0.0, 1.0);
    }
    else
    {
        out.o_color = float4(1.0, 0.0, 0.0, 1.0);
    }
    return out;
}

