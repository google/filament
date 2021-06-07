static float4 FP32Out;
static uint UNORM8;
static uint SNORM8;
static uint UNORM16;
static uint SNORM16;
static uint UNORM8Out;
static float4 FP32;
static uint SNORM8Out;
static uint UNORM16Out;
static uint SNORM16Out;

struct SPIRV_Cross_Input
{
    nointerpolation uint SNORM8 : TEXCOORD0;
    nointerpolation uint UNORM8 : TEXCOORD1;
    nointerpolation uint SNORM16 : TEXCOORD2;
    nointerpolation uint UNORM16 : TEXCOORD3;
    nointerpolation float4 FP32 : TEXCOORD4;
};

struct SPIRV_Cross_Output
{
    float4 FP32Out : SV_Target0;
    uint UNORM8Out : SV_Target1;
    uint SNORM8Out : SV_Target2;
    uint UNORM16Out : SV_Target3;
    uint SNORM16Out : SV_Target4;
};

uint spvPackUnorm4x8(float4 value)
{
    uint4 Packed = uint4(round(saturate(value) * 255.0));
    return Packed.x | (Packed.y << 8) | (Packed.z << 16) | (Packed.w << 24);
}

float4 spvUnpackUnorm4x8(uint value)
{
    uint4 Packed = uint4(value & 0xff, (value >> 8) & 0xff, (value >> 16) & 0xff, value >> 24);
    return float4(Packed) / 255.0;
}

uint spvPackSnorm4x8(float4 value)
{
    int4 Packed = int4(round(clamp(value, -1.0, 1.0) * 127.0)) & 0xff;
    return uint(Packed.x | (Packed.y << 8) | (Packed.z << 16) | (Packed.w << 24));
}

float4 spvUnpackSnorm4x8(uint value)
{
    int SignedValue = int(value);
    int4 Packed = int4(SignedValue << 24, SignedValue << 16, SignedValue << 8, SignedValue) >> 24;
    return clamp(float4(Packed) / 127.0, -1.0, 1.0);
}

uint spvPackUnorm2x16(float2 value)
{
    uint2 Packed = uint2(round(saturate(value) * 65535.0));
    return Packed.x | (Packed.y << 16);
}

float2 spvUnpackUnorm2x16(uint value)
{
    uint2 Packed = uint2(value & 0xffff, value >> 16);
    return float2(Packed) / 65535.0;
}

uint spvPackSnorm2x16(float2 value)
{
    int2 Packed = int2(round(clamp(value, -1.0, 1.0) * 32767.0)) & 0xffff;
    return uint(Packed.x | (Packed.y << 16));
}

float2 spvUnpackSnorm2x16(uint value)
{
    int SignedValue = int(value);
    int2 Packed = int2(SignedValue << 16, SignedValue) >> 16;
    return clamp(float2(Packed) / 32767.0, -1.0, 1.0);
}

void frag_main()
{
    FP32Out = spvUnpackUnorm4x8(UNORM8);
    FP32Out = spvUnpackSnorm4x8(SNORM8);
    float2 _21 = spvUnpackUnorm2x16(UNORM16);
    FP32Out = float4(_21.x, _21.y, FP32Out.z, FP32Out.w);
    float2 _26 = spvUnpackSnorm2x16(SNORM16);
    FP32Out = float4(_26.x, _26.y, FP32Out.z, FP32Out.w);
    UNORM8Out = spvPackUnorm4x8(FP32);
    SNORM8Out = spvPackSnorm4x8(FP32);
    UNORM16Out = spvPackUnorm2x16(FP32.xy);
    SNORM16Out = spvPackSnorm2x16(FP32.zw);
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    UNORM8 = stage_input.UNORM8;
    SNORM8 = stage_input.SNORM8;
    UNORM16 = stage_input.UNORM16;
    SNORM16 = stage_input.SNORM16;
    FP32 = stage_input.FP32;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FP32Out = FP32Out;
    stage_output.UNORM8Out = UNORM8Out;
    stage_output.SNORM8Out = SNORM8Out;
    stage_output.UNORM16Out = UNORM16Out;
    stage_output.SNORM16Out = SNORM16Out;
    return stage_output;
}
