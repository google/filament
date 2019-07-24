#pragma clang diagnostic ignored "-Wmissing-prototypes"

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

void packing_int8(device SSBO& ssbo)
{
    short i16 = 10;
    int i32 = 20;
    char2 i8_2 = as_type<char2>(i16);
    char4 i8_4 = as_type<char4>(i32);
    i16 = as_type<short>(i8_2);
    i32 = as_type<int>(i8_4);
    ssbo.i8[0] = i8_4.x;
    ssbo.i8[1] = i8_4.y;
    ssbo.i8[2] = i8_4.z;
    ssbo.i8[3] = i8_4.w;
}

void packing_uint8(device SSBO& ssbo)
{
    ushort u16 = 10u;
    uint u32 = 20u;
    uchar2 u8_2 = as_type<uchar2>(u16);
    uchar4 u8_4 = as_type<uchar4>(u32);
    u16 = as_type<ushort>(u8_2);
    u32 = as_type<uint>(u8_4);
    ssbo.u8[0] = u8_4.x;
    ssbo.u8[1] = u8_4.y;
    ssbo.u8[2] = u8_4.z;
    ssbo.u8[3] = u8_4.w;
}

void compute_int8(device SSBO& ssbo, thread int4& vColor, constant Push& registers, constant UBO& ubo, thread int4& FragColorInt)
{
    char4 tmp = char4(vColor);
    tmp += char4(registers.i8);
    tmp += char4(char(-40));
    tmp += char4(-50);
    tmp += char4(char(10), char(20), char(30), char(40));
    tmp += char4(ssbo.i8[4]);
    tmp += char4(ubo.i8);
    FragColorInt = int4(tmp);
}

void compute_uint8(device SSBO& ssbo, thread int4& vColor, constant Push& registers, constant UBO& ubo, thread uint4& FragColorUint)
{
    uchar4 tmp = uchar4(char4(vColor));
    tmp += uchar4(registers.u8);
    tmp += uchar4(uchar(216));
    tmp += uchar4(206);
    tmp += uchar4(uchar(10), uchar(20), uchar(30), uchar(40));
    tmp += uchar4(ssbo.u8[4]);
    tmp += uchar4(ubo.u8);
    FragColorUint = uint4(tmp);
}

fragment main0_out main0(main0_in in [[stage_in]], device SSBO& ssbo [[buffer(0)]], constant Push& registers [[buffer(1)]], constant UBO& ubo [[buffer(2)]])
{
    main0_out out = {};
    packing_int8(ssbo);
    packing_uint8(ssbo);
    compute_int8(ssbo, in.vColor, registers, ubo, out.FragColorInt);
    compute_uint8(ssbo, in.vColor, registers, ubo, out.FragColorUint);
    return out;
}

