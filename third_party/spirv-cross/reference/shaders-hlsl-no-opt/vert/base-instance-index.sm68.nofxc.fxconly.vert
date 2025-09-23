static float4 gl_Position;
static int gl_BaseInstanceARB;
struct SPIRV_Cross_Input
{
    uint gl_BaseInstanceARB : SV_StartInstanceLocation;
};

struct SPIRV_Cross_Output
{
    float4 gl_Position : SV_Position;
};

void vert_main()
{
    gl_Position = float(gl_BaseInstanceARB).xxxx;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    gl_BaseInstanceARB = stage_input.gl_BaseInstanceARB;
    vert_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    return stage_output;
}
