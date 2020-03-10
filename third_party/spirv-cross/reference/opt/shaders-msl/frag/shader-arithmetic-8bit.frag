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
    short _196 = short(10);
    int _197 = 20;
    char2 _201 = as_type<char2>(short(10));
    char2 _198 = _201;
    char4 _199 = as_type<char4>(20);
    _196 = as_type<short>(_201);
    _197 = as_type<int>(_199);
    ssbo.i8[0] = _199.x;
    ssbo.i8[1] = _199.y;
    ssbo.i8[2] = _199.z;
    ssbo.i8[3] = _199.w;
    ushort _220 = ushort(10);
    uint _221 = 20u;
    uchar2 _225 = as_type<uchar2>(ushort(10));
    uchar2 _222 = _225;
    uchar4 _223 = as_type<uchar4>(20u);
    _220 = as_type<ushort>(_225);
    _221 = as_type<uint>(_223);
    ssbo.u8[0] = _223.x;
    ssbo.u8[1] = _223.y;
    ssbo.u8[2] = _223.z;
    ssbo.u8[3] = _223.w;
    char4 _246 = char4(in.vColor);
    char4 _244 = _246;
    char4 _251 = _246 + char4(registers.i8);
    _244 = _251;
    char4 _254 = _251 + char4(-40);
    _244 = _254;
    char4 _256 = _254 + char4(-50);
    _244 = _256;
    char4 _258 = _256 + char4(char(10), char(20), char(30), char(40));
    _244 = _258;
    char4 _263 = _258 + char4(ssbo.i8[4]);
    _244 = _263;
    char4 _268 = _263 + char4(ubo.i8);
    _244 = _268;
    out.FragColorInt = int4(_268);
    uchar4 _274 = uchar4(_246);
    uchar4 _271 = _274;
    uchar4 _279 = _274 + uchar4(registers.u8);
    _271 = _279;
    uchar4 _282 = _279 + uchar4(216);
    _271 = _282;
    uchar4 _284 = _282 + uchar4(206);
    _271 = _284;
    uchar4 _286 = _284 + uchar4(uchar(10), uchar(20), uchar(30), uchar(40));
    _271 = _286;
    uchar4 _291 = _286 + uchar4(ssbo.u8[4]);
    _271 = _291;
    uchar4 _296 = _291 + uchar4(ubo.u8);
    _271 = _296;
    out.FragColorUint = uint4(_296);
    return out;
}

