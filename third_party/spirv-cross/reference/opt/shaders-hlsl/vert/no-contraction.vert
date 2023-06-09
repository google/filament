static float4 gl_Position;
static float4 vA;
static float4 vB;
static float4 vC;

struct SPIRV_Cross_Input
{
    float4 vA : TEXCOORD0;
    float4 vB : TEXCOORD1;
    float4 vC : TEXCOORD2;
};

struct SPIRV_Cross_Output
{
    float4 gl_Position : SV_Position;
};

void vert_main()
{
    precise float4 _15 = vA * vB;
    precise float4 _19 = vA + vB;
    precise float4 _23 = vA - vB;
    precise float4 _30 = _15 + vC;
    precise float4 _34 = _15 + _19;
    precise float4 _36 = _34 + _23;
    precise float4 _38 = _36 + _30;
    gl_Position = _38;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    vA = stage_input.vA;
    vB = stage_input.vB;
    vC = stage_input.vC;
    vert_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    return stage_output;
}
