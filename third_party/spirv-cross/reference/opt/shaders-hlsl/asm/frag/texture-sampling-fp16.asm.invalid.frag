Texture2D<float4> uTexture : register(t0);
SamplerState _uTexture_sampler : register(s0);

static min16float4 FragColor;
static min16float2 UV;

struct SPIRV_Cross_Input
{
    min16float2 UV : TEXCOORD0;
};

struct SPIRV_Cross_Output
{
    min16float4 FragColor : SV_Target0;
};

void frag_main()
{
    FragColor = min16float4(uTexture.Sample(_uTexture_sampler, UV));
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    UV = stage_input.UV;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
