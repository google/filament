static float gl_FragDepth;
struct SPIRV_Cross_Output
{
    float gl_FragDepth : SV_DepthLessEqual;
};

void frag_main()
{
    gl_FragDepth = 0.5f;
}

[earlydepthstencil]
SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_FragDepth = gl_FragDepth;
    return stage_output;
}
