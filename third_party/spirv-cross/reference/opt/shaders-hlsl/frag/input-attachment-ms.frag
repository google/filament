Texture2DMS<float4> uSubpass0 : register(t0);
Texture2DMS<float4> uSubpass1 : register(t1);

static float4 gl_FragCoord;
static int gl_SampleID;
static float4 FragColor;

struct SPIRV_Cross_Input
{
    float4 gl_FragCoord : SV_Position;
    uint gl_SampleID : SV_SampleIndex;
};

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

void frag_main()
{
    FragColor = (uSubpass0.Load(int2(gl_FragCoord.xy), 1) + uSubpass1.Load(int2(gl_FragCoord.xy), 2)) + uSubpass0.Load(int2(gl_FragCoord.xy), gl_SampleID);
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    gl_FragCoord = stage_input.gl_FragCoord;
    gl_SampleID = stage_input.gl_SampleID;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
