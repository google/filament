static int index;
static uint FragColor;

struct SPIRV_Cross_Input
{
    nointerpolation int index : TEXCOORD0;
};

struct SPIRV_Cross_Output
{
    uint FragColor : SV_Target0;
};

void frag_main()
{
    uint _17 = uint(index);
    FragColor = uint(WaveActiveMin(index));
    FragColor = uint(WaveActiveMax(int(_17)));
    FragColor = WaveActiveMin(uint(index));
    FragColor = WaveActiveMax(_17);
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    index = stage_input.index;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
