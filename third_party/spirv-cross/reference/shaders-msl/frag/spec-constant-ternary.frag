#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

constant uint s_tmp [[function_constant(0)]];
constant uint s = is_function_constant_defined(s_tmp) ? s_tmp : 10u;
constant bool _13 = (s > 20u);
constant uint f = _13 ? 30u : 50u;

struct main0_out
{
    float FragColor [[color(0)]];
};

fragment main0_out main0()
{
    main0_out out = {};
    out.FragColor = float(f);
    return out;
}

