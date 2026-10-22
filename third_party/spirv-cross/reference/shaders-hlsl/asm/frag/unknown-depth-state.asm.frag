Texture2D<float4> uShadow : register(t0);
SamplerComparisonState _uShadow_sampler : register(s0);
Texture2D<float4> uTexture : register(t1);
SamplerComparisonState uSampler : register(s2);

static float3 vUV;
static float FragColor;

struct SPIRV_Cross_Input
{
    float3 vUV : TEXCOORD0;
};

struct SPIRV_Cross_Output
{
    float FragColor : SV_Target0;
};

float sample_combined()
{
    return uShadow.SampleCmp(_uShadow_sampler, vUV.xy, vUV.z);
}

float sample_separate()
{
    return uTexture.SampleCmp(uSampler, vUV.xy, vUV.z);
}

void frag_main()
{
    FragColor = sample_combined() + sample_separate();
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    vUV = stage_input.vUV;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
