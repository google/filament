Buffer<float4> uFloatSampler : register(t1);
Buffer<int4> uIntSampler : register(t2);
Buffer<uint4> uUintSampler : register(t3);

static float4 gl_Position;
struct SPIRV_Cross_Output
{
    float4 gl_Position : SV_Position;
};

float4 sample_from_function(Buffer<float4> s0, Buffer<int4> s1, Buffer<uint4> s2)
{
    return (s0.Load(20) + asfloat(s1.Load(40))) + asfloat(s2.Load(60));
}

void vert_main()
{
    gl_Position = sample_from_function(uFloatSampler, uIntSampler, uUintSampler);
}

SPIRV_Cross_Output main()
{
    vert_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    return stage_output;
}
