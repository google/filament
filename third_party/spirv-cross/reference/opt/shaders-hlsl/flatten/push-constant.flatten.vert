uniform float4 PushMe[6];

static float4 gl_Position;
static float4 Pos;
static float2 vRot;
static float2 Rot;

struct SPIRV_Cross_Input
{
    float2 Rot : TEXCOORD0;
    float4 Pos : TEXCOORD1;
};

struct SPIRV_Cross_Output
{
    float2 vRot : TEXCOORD0;
    float4 gl_Position : SV_Position;
};

void vert_main()
{
    gl_Position = mul(Pos, float4x4(PushMe[0], PushMe[1], PushMe[2], PushMe[3]));
    vRot = mul(Rot, float2x2(PushMe[4].xy, PushMe[4].zw)) + PushMe[5].z.xx;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    Pos = stage_input.Pos;
    Rot = stage_input.Rot;
    vert_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    stage_output.vRot = vRot;
    return stage_output;
}
