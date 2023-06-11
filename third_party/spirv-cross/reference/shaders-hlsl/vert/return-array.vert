static const float4 _20[2] = { 10.0f.xxxx, 20.0f.xxxx };

static float4 gl_Position;
static float4 vInput0;
static float4 vInput1;

struct SPIRV_Cross_Input
{
    float4 vInput0 : TEXCOORD0;
    float4 vInput1 : TEXCOORD1;
};

struct SPIRV_Cross_Output
{
    float4 gl_Position : SV_Position;
};

void test(out float4 spvReturnValue[2])
{
    spvReturnValue = _20;
}

void test2(out float4 spvReturnValue[2])
{
    float4 foobar[2];
    foobar[0] = vInput0;
    foobar[1] = vInput1;
    spvReturnValue = foobar;
}

void vert_main()
{
    float4 _42[2];
    test(_42);
    float4 _44[2];
    test2(_44);
    gl_Position = _42[0] + _44[1];
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    vInput0 = stage_input.vInput0;
    vInput1 = stage_input.vInput1;
    vert_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    return stage_output;
}
