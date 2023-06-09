uniform int4 UBO1[2];
uniform uint4 UBO2[2];
uniform float4 UBO0[2];

static float4 FragColor;

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

void frag_main()
{
    FragColor = ((((float4(UBO1[0]) + float4(UBO1[1])) + float4(UBO2[0])) + float4(UBO2[1])) + UBO0[0]) + UBO0[1];
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
