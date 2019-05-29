#ifndef SPIRV_CROSS_CONSTANT_ID_10
#define SPIRV_CROSS_CONSTANT_ID_10 2
#endif
static const int Value = SPIRV_CROSS_CONSTANT_ID_10;

cbuffer SpecConstArray : register(b0)
{
    float4 _15_samples[Value] : packoffset(c0);
};


static float4 FragColor;
static int Index;

struct SPIRV_Cross_Input
{
    nointerpolation int Index : TEXCOORD0;
};

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

void frag_main()
{
    FragColor = _15_samples[Index];
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    Index = stage_input.Index;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
