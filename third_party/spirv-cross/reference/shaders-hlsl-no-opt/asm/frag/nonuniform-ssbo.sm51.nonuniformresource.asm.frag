RWByteAddressBuffer ssbos[] : register(u3, space0);

static int vIndex;
static float4 FragColor;

struct SPIRV_Cross_Input
{
    nointerpolation int vIndex : TEXCOORD0;
};

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

void frag_main()
{
    int i = vIndex;
    int _42 = i + 60;
    int _45 = i + 70;
    ssbos[NonUniformResourceIndex(_42)].Store4(_45 * 16 + 16, asuint(20.0f.xxxx));
    int _48 = i + 100;
    uint _49;
    ssbos[NonUniformResourceIndex(_48)].InterlockedAdd(0, 100u, _49);
    int _51 = i;
    uint _52;
    ssbos[NonUniformResourceIndex(_51)].GetDimensions(_52);
    _52 = (_52 - 16) / 16;
    FragColor.z += float(int(_52));
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    vIndex = stage_input.vIndex;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
