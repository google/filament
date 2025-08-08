#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    int oA [[color(0)]];
    uint oB [[color(1)]];
};

struct main0_in
{
    int A [[user(locn0)]];
    uint B [[user(locn1)]];
    int C [[user(locn2)]];
    uint D [[user(locn3)]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    out.oB = uint(in.A - int(in.B) * (in.A / int(in.B)));
    out.oB = uint(in.A - in.C * (in.A / in.C));
    out.oB = uint(int(in.B) - int(in.D) * (int(in.B) / int(in.D)));
    out.oB = uint(int(in.B) - in.A * (int(in.B) / in.A));
    out.oA = in.A - int(in.B) * (in.A / int(in.B));
    out.oA = in.A - in.C * (in.A / in.C);
    out.oA = int(in.B) - int(in.D) * (int(in.B) / int(in.D));
    out.oA = int(in.B) - in.A * (int(in.B) / in.A);
    return out;
}

