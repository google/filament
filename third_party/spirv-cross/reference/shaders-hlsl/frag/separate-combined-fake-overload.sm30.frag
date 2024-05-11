uniform sampler2D uSamp;
uniform sampler2D SPIRV_Cross_CombineduTuS;

static float4 FragColor;

struct SPIRV_Cross_Output
{
    float4 FragColor : COLOR0;
};

float4 samp(sampler2D uSamp_1)
{
    return tex2D(uSamp_1, 0.5f.xx);
}

float4 samp_1(sampler2D SPIRV_Cross_CombinedTS)
{
    return tex2D(SPIRV_Cross_CombinedTS, 0.5f.xx);
}

void frag_main()
{
    FragColor = samp(uSamp) + samp_1(SPIRV_Cross_CombineduTuS);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = float4(FragColor);
    return stage_output;
}
