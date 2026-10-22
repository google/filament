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
    FragColor = float(IsHelperLane());
    discard;
    bool _16 = IsHelperLane();
    FragColor = float(_16);
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
