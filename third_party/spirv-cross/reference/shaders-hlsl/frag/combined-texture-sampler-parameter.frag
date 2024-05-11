Texture2D<float4> uSampler : register(t0);
SamplerState _uSampler_sampler : register(s0);
Texture2D<float4> uSamplerShadow : register(t1);
SamplerComparisonState _uSamplerShadow_sampler : register(s1);

static float FragColor;

struct SPIRV_Cross_Output
{
    float FragColor : SV_Target0;
};

float4 samp2(Texture2D<float4> s, SamplerState _s_sampler)
{
    return s.Sample(_s_sampler, 1.0f.xx) + s.Load(int3(int2(10, 10), 0));
}

float4 samp3(Texture2D<float4> s, SamplerState _s_sampler)
{
    return samp2(s, _s_sampler);
}

float samp4(Texture2D<float4> s, SamplerComparisonState _s_sampler)
{
    return s.SampleCmp(_s_sampler, 1.0f.xxx.xy, 1.0f);
}

float samp(Texture2D<float4> s0, SamplerState _s0_sampler, Texture2D<float4> s1, SamplerComparisonState _s1_sampler)
{
    return samp3(s0, _s0_sampler).x + samp4(s1, _s1_sampler);
}

void frag_main()
{
    FragColor = samp(uSampler, _uSampler_sampler, uSamplerShadow, _uSamplerShadow_sampler);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
