static float4 gl_Position;
static int gl_BaseVertexARB;
cbuffer SPIRV_Cross_VertexInfo
{
    int SPIRV_Cross_BaseVertex;
    int SPIRV_Cross_BaseInstance;
};

struct SPIRV_Cross_Input
{
};

struct SPIRV_Cross_Output
{
    float4 gl_Position : SV_Position;
};

void vert_main()
{
    gl_Position = float(gl_BaseVertexARB).xxxx;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    gl_BaseVertexARB = SPIRV_Cross_BaseVertex;
    vert_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    return stage_output;
}
