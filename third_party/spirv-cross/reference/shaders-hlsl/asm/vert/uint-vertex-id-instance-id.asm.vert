static float4 gl_Position;
static int gl_VertexIndex;
static int gl_InstanceIndex;
struct SPIRV_Cross_Input
{
    uint gl_VertexIndex : SV_VertexID;
    uint gl_InstanceIndex : SV_InstanceID;
};

struct SPIRV_Cross_Output
{
    float4 gl_Position : SV_Position;
};

float4 _main(uint vid, uint iid)
{
    return float(vid + iid).xxxx;
}

void vert_main()
{
    uint vid = uint(gl_VertexIndex);
    uint iid = uint(gl_InstanceIndex);
    uint param = vid;
    uint param_1 = iid;
    gl_Position = _main(param, param_1);
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    gl_VertexIndex = int(stage_input.gl_VertexIndex);
    gl_InstanceIndex = int(stage_input.gl_InstanceIndex);
    vert_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    return stage_output;
}
