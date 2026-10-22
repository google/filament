Texture2DMS<float4> uTex : register(t0);
SamplerState _uTex_sampler : register(s0);

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
    int2 _22 = int2(gl_FragCoord.xy);
    FragColor = uTex.Load(_22, 0);
    FragColor += uTex.Load(_22, 1);
    FragColor += uTex.Load(_22, 2);
    FragColor += uTex.Load(_22, 3);
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    gl_FragCoord = stage_input.gl_FragCoord;
    gl_FragCoord.w = 1.0 / gl_FragCoord.w;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
