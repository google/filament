Texture2D<float4> uDepth : register(t2);
SamplerComparisonState uSampler : register(s0);
SamplerState uSampler1 : register(s1);

static float FragColor;

struct SPIRV_Cross_Output
{
    float FragColor : SV_Target0;
};

float samp2(Texture2D<float4> t, SamplerComparisonState s)
{
    return t.SampleCmp(s, 1.0f.xxx.xy, 1.0f.xxx.z);
}

float samp3(Texture2D<float4> t, SamplerState s)
{
    return t.Sample(s, 1.0f.xx).x;
}

float samp(Texture2D<float4> t, SamplerComparisonState s, SamplerState s1)
{
    float r0 = samp2(t, s);
    float r1 = samp3(t, s1);
    return r0 + r1;
}

void frag_main()
{
    FragColor = samp(uDepth, uSampler, uSampler1);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
