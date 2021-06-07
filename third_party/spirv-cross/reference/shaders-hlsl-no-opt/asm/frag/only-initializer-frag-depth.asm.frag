static float gl_FragDepth = 0.5f;
struct SPIRV_Cross_Output
{
    float gl_FragDepth : SV_Depth;
};

void frag_main()
{
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_FragDepth = gl_FragDepth;
    return stage_output;
}
