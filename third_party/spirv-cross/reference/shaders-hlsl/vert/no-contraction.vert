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
    float4 mul = _15;
    precise float4 _19 = vA + vB;
    float4 add = _19;
    precise float4 _23 = vA - vB;
    float4 sub = _23;
    precise float4 _27 = vA * vB;
    precise float4 _30 = _27 + vC;
    float4 mad = _30;
    precise float4 _34 = mul + add;
    precise float4 _36 = _34 + sub;
    precise float4 _38 = _36 + mad;
    float4 summed = _38;
    gl_Position = summed;
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
