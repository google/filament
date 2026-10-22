static float4 gl_Position;
static float4x4 m4;
static float4 v;
static float3x3 m3;
static float2x2 m2;

struct SPIRV_Cross_Input
{
    float4 m4_0 : TEXCOORD0;
    float4 m4_1 : TEXCOORD1;
    float4 m4_2 : TEXCOORD2;
    float4 m4_3 : TEXCOORD3;
    float3 m3_0 : TEXCOORD4;
    float3 m3_1 : TEXCOORD5;
    float3 m3_2 : TEXCOORD6;
    float2 m2_0 : TEXCOORD7;
    float2 m2_1 : TEXCOORD8;
    float4 v : TEXCOORD9;
};

struct SPIRV_Cross_Output
{
    float4 gl_Position : SV_Position;
};

void vert_main()
{
    gl_Position = mul(v, m4);
    float4 _35 = gl_Position;
    float3 _37 = _35.xyz + mul(v.xyz, m3);
    gl_Position.x = _37.x;
    gl_Position.y = _37.y;
    gl_Position.z = _37.z;
    float4 _56 = gl_Position;
    float2 _58 = _56.xy + mul(v.xy, m2);
    gl_Position.x = _58.x;
    gl_Position.y = _58.y;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    m4[0] = stage_input.m4_0;
    m4[1] = stage_input.m4_1;
    m4[2] = stage_input.m4_2;
    m4[3] = stage_input.m4_3;
    v = stage_input.v;
    m3[0] = stage_input.m3_0;
    m3[1] = stage_input.m3_1;
    m3[2] = stage_input.m3_2;
    m2[0] = stage_input.m2_0;
    m2[1] = stage_input.m2_1;
    vert_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    return stage_output;
}
