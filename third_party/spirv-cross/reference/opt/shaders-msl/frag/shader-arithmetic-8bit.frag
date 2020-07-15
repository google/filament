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
    char2 _202 = as_type<char2>(short(10));
    char2 _198 = _202;
    char4 _199 = as_type<char4>(20);
    _196 = as_type<short>(_202);
    _197 = as_type<int>(_199);
    ssbo.i8[0] = _199.x;
    ssbo.i8[1] = _199.y;
    ssbo.i8[2] = _199.z;
    ssbo.i8[3] = _199.w;
    ushort _221 = ushort(10);
    uint _222 = 20u;
    uchar2 _227 = as_type<uchar2>(ushort(10));
    uchar2 _223 = _227;
    uchar4 _224 = as_type<uchar4>(20u);
    _221 = as_type<ushort>(_227);
    _222 = as_type<uint>(_224);
    ssbo.u8[0] = _224.x;
    ssbo.u8[1] = _224.y;
    ssbo.u8[2] = _224.z;
    ssbo.u8[3] = _224.w;
    char4 _249 = char4(in.vColor);
    char4 _246 = _249;
    char4 _254 = _249 + char4(registers.i8);
    _246 = _254;
    char4 _257 = _254 + char4(-40);
    _246 = _257;
    char4 _259 = _257 + char4(-50);
    _246 = _259;
    char4 _261 = _259 + char4(char(10), char(20), char(30), char(40));
    _246 = _261;
    char4 _266 = _261 + char4(ssbo.i8[4]);
    _246 = _266;
    char4 _271 = _266 + char4(ubo.i8);
    _246 = _271;
    out.FragColorInt = int4(_271);
    uchar4 _278 = uchar4(_249);
    uchar4 _274 = _278;
    uchar4 _283 = _278 + uchar4(registers.u8);
    _274 = _283;
    uchar4 _286 = _283 + uchar4(216);
    _274 = _286;
    uchar4 _288 = _286 + uchar4(206);
    _274 = _288;
    uchar4 _290 = _288 + uchar4(uchar(10), uchar(20), uchar(30), uchar(40));
    _274 = _290;
    uchar4 _295 = _290 + uchar4(ssbo.u8[4]);
    _274 = _295;
    uchar4 _300 = _295 + uchar4(ubo.u8);
    _274 = _300;
    out.FragColorUint = uint4(_300);
    return out;
}

