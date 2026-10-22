Buffer<float4> uSamp : register(t4);
RWBuffer<float4> uSampo : register(u5);

static float4 gl_Position;
struct SPIRV_Cross_Output
{
    float4 gl_Position : SV_Position;
};

void vert_main()
{
    gl_Position = uSamp.Load(10) + uSampo[100];
}

SPIRV_Cross_Output main()
{
    vert_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    return stage_output;
}
