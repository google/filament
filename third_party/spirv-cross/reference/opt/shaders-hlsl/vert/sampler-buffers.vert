Buffer<float4> uFloatSampler : register(t1);
Buffer<int4> uIntSampler : register(t2);
Buffer<uint4> uUintSampler : register(t3);

static float4 gl_Position;
struct SPIRV_Cross_Output
{
    float4 gl_Position : SV_Position;
};

void vert_main()
{
    gl_Position = (uFloatSampler.Load(20) + asfloat(uIntSampler.Load(40))) + asfloat(uUintSampler.Load(60));
}

SPIRV_Cross_Output main()
{
    vert_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    return stage_output;
}
