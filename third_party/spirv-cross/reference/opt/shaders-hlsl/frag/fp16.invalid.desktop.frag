static min16float4 v4;
static min16float3 v3;
static min16float v1;
static min16float2 v2;
static float o1;
static float2 o2;
static float3 o3;
static float4 o4;

struct SPIRV_Cross_Input
{
    min16float v1 : TEXCOORD0;
    min16float2 v2 : TEXCOORD1;
    min16float3 v3 : TEXCOORD2;
    min16float4 v4 : TEXCOORD3;
};

struct SPIRV_Cross_Output
{
    float o1 : SV_Target0;
    float2 o2 : SV_Target1;
    float3 o3 : SV_Target2;
    float4 o4 : SV_Target3;
};

void frag_main()
{
    min16float4 _324;
    min16float4 _387 = modf(v4, _324);
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    v4 = stage_input.v4;
    v3 = stage_input.v3;
    v1 = stage_input.v1;
    v2 = stage_input.v2;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.o1 = o1;
    stage_output.o2 = o2;
    stage_output.o3 = o3;
    stage_output.o4 = o4;
    return stage_output;
}
