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
    int _23 = i + 60;
    int _28 = i + 70;
    ssbos[NonUniformResourceIndex(_23)].Store4(_28 * 16 + 16, asuint(20.0f.xxxx));
    int _36 = i + 100;
    uint _43;
    ssbos[NonUniformResourceIndex(_36)].InterlockedAdd(0, 100u, _43);
    int _47 = i;
    uint _50;
    ssbos[NonUniformResourceIndex(_47)].GetDimensions(_50);
    _50 = (_50 - 16) / 16;
    FragColor.z += float(int(_50));
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    vIndex = stage_input.vIndex;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
