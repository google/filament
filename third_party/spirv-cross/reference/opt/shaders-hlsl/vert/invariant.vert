static float4 gl_Position;
static float4 vInput0;
static float4 vInput1;
static float4 vInput2;
static float4 vColor;

struct SPIRV_Cross_Input
{
    float4 vInput0 : TEXCOORD0;
    float4 vInput1 : TEXCOORD1;
    float4 vInput2 : TEXCOORD2;
};

struct SPIRV_Cross_Output
{
    precise float4 vColor : TEXCOORD0;
    precise float4 gl_Position : SV_Position;
};

void vert_main()
{
    float4 _21 = mad(vInput1, vInput2, vInput0);
    gl_Position = _21;
    float4 _27 = vInput0 - vInput1;
    float4 _29 = _27 * vInput2;
    vColor = _29;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    vInput0 = stage_input.vInput0;
    vInput1 = stage_input.vInput1;
    vInput2 = stage_input.vInput2;
    vert_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    stage_output.vColor = vColor;
    return stage_output;
}
