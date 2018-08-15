Texture2D<float4> uTexture : register(t0);
SamplerState _uTexture_sampler : register(s0);

static float4 gl_FragCoord;
static float4 FragColor;

struct SPIRV_Cross_Input
{
    float4 gl_FragCoord : SV_Position;
};

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

void frag_main()
{
    FragColor = uTexture.Load(int3(int2(gl_FragCoord.xy), 0), int2(1, 1));
    FragColor += uTexture.Load(int3(int2(gl_FragCoord.xy), 0), int2(-1, 1));
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    gl_FragCoord = stage_input.gl_FragCoord;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
