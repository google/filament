static float4 FragColor;
static float4 vIn;
static int4 vIn1;

struct SPIRV_Cross_Input
{
    float4 vIn : TEXCOORD0;
    nointerpolation int4 vIn1 : TEXCOORD1;
};

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

void frag_main()
{
    FragColor = -(-vIn);
    int4 a = ~(~vIn1);
    bool b = false;
    b = !(!b);
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    vIn = stage_input.vIn;
    vIn1 = stage_input.vIn1;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
