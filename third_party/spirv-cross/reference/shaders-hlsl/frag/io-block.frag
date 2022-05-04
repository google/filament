struct VertexOut
{
    float4 a;
    float4 b;
};

static float4 FragColor;
static VertexOut _12;

struct SPIRV_Cross_Input
{
    float4 VertexOut_a : TEXCOORD1;
    float4 VertexOut_b : TEXCOORD2;
};

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

void frag_main()
{
    FragColor = _12.a + _12.b;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    _12.a = stage_input.VertexOut_a;
    _12.b = stage_input.VertexOut_b;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
