#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct type_Globals
{
    float4 _BorderWidths[4];
};

struct main0_out
{
    float4 out_var_SV_Target [[color(0)]];
};

fragment main0_out main0(constant type_Globals& _Globals [[buffer(0)]], float4 gl_FragCoord [[position]])
{
    main0_out out = {};
    float2 _31 = float2(_Globals._BorderWidths[0].x, _Globals._BorderWidths[1].x);
    float2 _39;
    if (gl_FragCoord.x > 0.0)
    {
        float2 _38 = _31;
        _38.x = _Globals._BorderWidths[2].x;
        _39 = _38;
    }
    else
    {
        _39 = _31;
    }
    out.out_var_SV_Target = float4(_39, 0.0, 1.0);
    return out;
}

