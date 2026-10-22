Texture2D<float4> uDepth : register(t2);
SamplerComparisonState uSampler : register(s0);
SamplerState uSampler1 : register(s1);

static float FragColor;

struct SPIRV_Cross_Output
{
    float FragColor : SV_Target0;
};

void frag_main()
{
    FragColor = uDepth.SampleCmp(uSampler, 1.0f.xxx.xy, 1.0f) + uDepth.Sample(uSampler1, 1.0f.xx).x;
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
