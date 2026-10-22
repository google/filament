uniform float4 UBO[4];

static float3 FragColor;
static float3 vNormal;

struct SPIRV_Cross_Input
{
    nointerpolation float3 vNormal : TEXCOORD0;
};

struct SPIRV_Cross_Output
{
    float3 FragColor : SV_Target0;
};

void frag_main()
{
    float4x4 _19 = float4x4(UBO[0], UBO[1], UBO[2], UBO[3]);
    FragColor = mul(vNormal, float3x3(_19[0].xyz, _19[1].xyz, _19[2].xyz));
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    vNormal = stage_input.vNormal;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
