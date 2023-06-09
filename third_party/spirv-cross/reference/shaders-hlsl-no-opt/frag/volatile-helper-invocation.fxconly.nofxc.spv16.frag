static float FragColor;

struct SPIRV_Cross_Input
{
};

struct SPIRV_Cross_Output
{
    float FragColor : SV_Target0;
};

void frag_main()
{
    bool _12 = IsHelperLane();
    float _15 = float(_12);
    FragColor = _15;
    discard;
    bool _16 = IsHelperLane();
    float _17 = float(_16);
    FragColor = _17;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
