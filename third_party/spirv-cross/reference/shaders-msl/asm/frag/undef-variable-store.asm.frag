#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

constant float4 _38 = {};
constant float4 _47 = {};

struct main0_out
{
    float4 _entryPointOutput [[color(0)]];
};

fragment main0_out main0()
{
    main0_out out = {};
    float4 _27;
    do
    {
        float2 _26 = float2(0.0);
        if (_26.x != 0.0)
        {
            _27 = float4(1.0, 0.0, 0.0, 1.0);
            break;
        }
        else
        {
            _27 = float4(1.0, 1.0, 0.0, 1.0);
            break;
        }
        _27 = _38;
        break;
    } while (false);
    out._entryPointOutput = _27;
    return out;
}

