cbuffer UBO : register(b0)
{
    row_major float4x4 _13_m : packoffset(c1);
    float4 _13_v : packoffset(c0);
};


static float4 FragColor;
static float4 vColor;

struct SPIRV_Cross_Input
{
    float4 vColor : TEXCOORD0;
};

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

void frag_main()
{
    FragColor = mul(vColor, _13_m) + _13_v;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    vColor = stage_input.vColor;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
