static float4 a4;
static float4 b4;
static float3 a3;
static float3 b3;
static float2 a2;
static float2 b2;
static float a1;
static float b1;
static float4 FragColor;

struct SPIRV_Cross_Input
{
    float4 a4 : TEXCOORD0;
    float3 a3 : TEXCOORD1;
    float2 a2 : TEXCOORD2;
    float a1 : TEXCOORD3;
    float4 b4 : TEXCOORD4;
    float3 b3 : TEXCOORD5;
    float2 b2 : TEXCOORD6;
    float b1 : TEXCOORD7;
};

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

float mod(float x, float y)
{
    return x - y * floor(x / y);
}

float2 mod(float2 x, float2 y)
{
    return x - y * floor(x / y);
}

float3 mod(float3 x, float3 y)
{
    return x - y * floor(x / y);
}

float4 mod(float4 x, float4 y)
{
    return x - y * floor(x / y);
}

void frag_main()
{
    FragColor = ((mod(a4, b4) + mod(a3, b3).xyzx) + mod(a2, b2).xyxy) + mod(a1, b1).xxxx;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    a4 = stage_input.a4;
    b4 = stage_input.b4;
    a3 = stage_input.a3;
    b3 = stage_input.b3;
    a2 = stage_input.a2;
    b2 = stage_input.b2;
    a1 = stage_input.a1;
    b1 = stage_input.b1;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
