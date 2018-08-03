Texture2D<float4> uSubpass0 : register(t0);
Texture2D<float4> uSubpass1 : register(t1);

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

float4 load_subpasses(Texture2D<float4> uInput)
{
    return uInput.Load(int3(int2(gl_FragCoord.xy), 0));
}

void frag_main()
{
    FragColor = uSubpass0.Load(int3(int2(gl_FragCoord.xy), 0)) + load_subpasses(uSubpass1);
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    gl_FragCoord = stage_input.gl_FragCoord;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
