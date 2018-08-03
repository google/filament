static float2 FP32Out;
static uint FP16;
static uint FP16Out;
static float2 FP32;

struct SPIRV_Cross_Input
{
    nointerpolation uint FP16 : TEXCOORD0;
    nointerpolation float2 FP32 : TEXCOORD1;
};

struct SPIRV_Cross_Output
{
    float2 FP32Out : SV_Target0;
    uint FP16Out : SV_Target1;
};

uint SPIRV_Cross_packHalf2x16(float2 value)
{
    uint2 Packed = f32tof16(value);
    return Packed.x | (Packed.y << 16);
}

float2 SPIRV_Cross_unpackHalf2x16(uint value)
{
    return f16tof32(uint2(value & 0xffff, value >> 16));
}

void frag_main()
{
    FP32Out = SPIRV_Cross_unpackHalf2x16(FP16);
    FP16Out = SPIRV_Cross_packHalf2x16(FP32);
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    FP16 = stage_input.FP16;
    FP32 = stage_input.FP32;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FP32Out = FP32Out;
    stage_output.FP16Out = FP16Out;
    return stage_output;
}
