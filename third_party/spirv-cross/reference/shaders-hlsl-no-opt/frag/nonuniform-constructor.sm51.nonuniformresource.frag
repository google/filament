Texture2D<float4> uTex[] : register(t0, space0);
SamplerState Immut : register(s0, space1);

static float4 FragColor;
static int vIndex;
static float2 vUV;

struct SPIRV_Cross_Input
{
    float2 vUV : TEXCOORD0;
    nointerpolation int vIndex : TEXCOORD1;
};

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

void frag_main()
{
    FragColor = uTex[NonUniformResourceIndex(vIndex)].Sample(Immut, vUV);
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    vIndex = stage_input.vIndex;
    vUV = stage_input.vUV;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
