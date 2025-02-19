#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

constant float4 _48 = {};
constant float4 _31 = {};

struct main0_out
{
    float4 _entryPointOutput [[color(0)]];
};

fragment main0_out main0()
{
    main0_out out = {};
    float4 _37;
    do
    {
        float2 _35 = float2(0.0);
        if (_35.x != 0.0)
        {
            _37 = float4(1.0, 0.0, 0.0, 1.0);
            break;
        }
        else
        {
            _37 = float4(1.0, 1.0, 0.0, 1.0);
            break;
        }
        _37 = _48;
        break;
    } while (false);
    out._entryPointOutput = _37;
    return out;
}

