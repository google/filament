ByteAddressBuffer _8 : register(t0, space2);
Texture2D<float4> uSamplers[] : register(t0, space0);
SamplerState _uSamplers_sampler[] : register(s0, space0);
Texture2D<float4> uSampler : register(t1, space1);
SamplerState _uSampler_sampler : register(s1, space1);

static float4 gl_FragCoord;
static float4 FragColor;
static float2 vUV;

struct SPIRV_Cross_Input
{
    float2 vUV : TEXCOORD0;
    float4 gl_FragCoord : SV_Position;
};

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

void frag_main()
{
    FragColor = uSamplers[NonUniformResourceIndex(_8.Load(40))].SampleLevel(_uSamplers_sampler[NonUniformResourceIndex(_8.Load(40))], vUV, 0.0f);
    FragColor += uSampler.SampleLevel(_uSampler_sampler, vUV, float(_8.Load(int(gl_FragCoord.y) * 4 + 0)));
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    gl_FragCoord = stage_input.gl_FragCoord;
    gl_FragCoord.w = 1.0 / gl_FragCoord.w;
    vUV = stage_input.vUV;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
