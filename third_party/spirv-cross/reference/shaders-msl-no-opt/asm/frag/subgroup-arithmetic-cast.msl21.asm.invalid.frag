#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    uint FragColor [[color(0)]];
};

struct main0_in
{
    int index [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    uint _17 = uint(in.index);
    out.FragColor = uint(simd_min(in.index));
    out.FragColor = uint(simd_max(int(_17)));
    out.FragColor = simd_min(uint(in.index));
    out.FragColor = simd_max(_17);
    out.FragColor = uint(quad_min(in.index));
    out.FragColor = uint(quad_max(int(_17)));
    out.FragColor = quad_min(uint(in.index));
    out.FragColor = quad_max(_17);
    return out;
}

