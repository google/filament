static float4 FragColor;

struct VertexOut
{
    float4 a : TEXCOORD1;
    float4 b : TEXCOORD2;
};

static VertexOut _12;

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

void frag_main()
{
    FragColor = _12.a + _12.b;
}

SPIRV_Cross_Output main(in VertexOut stage_input_12)
{
    _12 = stage_input_12;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
