static float4 Float;
static float vFloat;
static int4 Int;
static int vInt;
static float4 Float2;
static int4 Int2;

struct SPIRV_Cross_Input
{
    nointerpolation float vFloat : TEXCOORD0;
    nointerpolation int vInt : TEXCOORD1;
};

struct SPIRV_Cross_Output
{
    float4 Float : SV_Target0;
    int4 Int : SV_Target1;
    float4 Float2 : SV_Target2;
    int4 Int2 : SV_Target3;
};

void frag_main()
{
    Float = vFloat.xxxx * 2.0f;
    Int = vInt.xxxx * int4(2, 2, 2, 2);
    Float2 = 10.0f.xxxx;
    Int2 = int4(10, 10, 10, 10);
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    vFloat = stage_input.vFloat;
    vInt = stage_input.vInt;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.Float = Float;
    stage_output.Int = Int;
    stage_output.Float2 = Float2;
    stage_output.Int2 = Int2;
    return stage_output;
}
