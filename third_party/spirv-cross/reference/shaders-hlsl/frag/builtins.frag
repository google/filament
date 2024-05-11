static float4 gl_FragCoord;
static float gl_FragDepth;
static float4 FragColor;
static float4 vColor;

struct SPIRV_Cross_Input
{
    float4 vColor : TEXCOORD0;
    float4 gl_FragCoord : SV_Position;
};

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
    float gl_FragDepth : SV_Depth;
};

void frag_main()
{
    FragColor = gl_FragCoord + vColor;
    gl_FragDepth = 0.5f;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    gl_FragCoord = stage_input.gl_FragCoord;
    gl_FragCoord.w = 1.0 / gl_FragCoord.w;
    vColor = stage_input.vColor;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_FragDepth = gl_FragDepth;
    stage_output.FragColor = FragColor;
    return stage_output;
}
