static float4 gl_Position;
static float gl_ClipDistance[2];
static float gl_CullDistance[1];
struct SPIRV_Cross_Output
{
    float4 gl_Position : SV_Position;
    float2 gl_ClipDistance0 : SV_ClipDistance0;
    float gl_CullDistance0 : SV_CullDistance0;
};

void vert_main()
{
    gl_Position = 1.0f.xxxx;
    gl_ClipDistance[0] = 0.0f;
    gl_ClipDistance[1] = 0.0f;
    gl_CullDistance[0] = 4.0f;
}

SPIRV_Cross_Output main()
{
    vert_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    stage_output.gl_ClipDistance0.x = gl_ClipDistance[0];
    stage_output.gl_ClipDistance0.y = gl_ClipDistance[1];
    stage_output.gl_CullDistance0.x = gl_CullDistance[0];
    return stage_output;
}
