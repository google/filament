static float4 gl_Position;
static float4x4 m;
static float3 pos;

struct SPIRV_Cross_Input
{
    float3 pos : TEXCOORD0;
    float4 m_0 : TEXCOORD1_0;
    float4 m_1 : TEXCOORD1_1;
    float4 m_2 : TEXCOORD1_2;
    float4 m_3 : TEXCOORD1_3;
};

struct SPIRV_Cross_Output
{
    float4 gl_Position : SV_Position;
};

void vert_main()
{
    gl_Position = mul(float4(pos, 1.0f), m);
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    m[0] = stage_input.m_0;
    m[1] = stage_input.m_1;
    m[2] = stage_input.m_2;
    m[3] = stage_input.m_3;
    pos = stage_input.pos;
    vert_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    return stage_output;
}
