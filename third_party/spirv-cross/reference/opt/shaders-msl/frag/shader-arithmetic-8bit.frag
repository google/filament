#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct SSBO
{
    char i8[16];
    uchar u8[16];
};

struct Push
{
    char i8;
    uchar u8;
};

struct UBO
{
    char i8;
    uchar u8;
};

struct main0_out
{
    int4 FragColorInt [[color(0)]];
    uint4 FragColorUint [[color(1)]];
};

struct main0_in
{
    int4 vColor [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]], device SSBO& ssbo [[buffer(0)]], constant Push& registers [[buffer(1)]], constant UBO& ubo [[buffer(2)]])
{
    main0_out out = {};
    char4 _204 = as_type<char4>(20);
    ssbo.i8[0] = _204.x;
    ssbo.i8[1] = _204.y;
    ssbo.i8[2] = _204.z;
    ssbo.i8[3] = _204.w;
    uchar4 _229 = as_type<uchar4>(20u);
    ssbo.u8[0] = _229.x;
    ssbo.u8[1] = _229.y;
    ssbo.u8[2] = _229.z;
    ssbo.u8[3] = _229.w;
    char4 _249 = char4(in.vColor);
    out.FragColorInt = int4((((((_249 + char4(registers.i8)) + char4(-40)) + char4(-50)) + char4(char(10), char(20), char(30), char(40))) + char4(ssbo.i8[4])) + char4(ubo.i8));
    out.FragColorUint = uint4((((((uchar4(_249) + uchar4(registers.u8)) + uchar4(216)) + uchar4(206)) + uchar4(uchar(10), uchar(20), uchar(30), uchar(40))) + uchar4(ssbo.u8[4])) + uchar4(ubo.u8));
    return out;
}

