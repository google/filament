static float gl_ClipDistance[2];
static float gl_CullDistance[1];
static float FragColor;

struct SPIRV_Cross_Input
{
    float2 gl_ClipDistance0 : SV_ClipDistance0;
    float gl_CullDistance0 : SV_CullDistance0;
};

struct SPIRV_Cross_Output
{
    float FragColor : SV_Target0;
};

void frag_main()
{
    FragColor = (gl_ClipDistance[0] + gl_CullDistance[0]) + gl_ClipDistance[1];
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    gl_ClipDistance[0] = stage_input.gl_ClipDistance0.x;
    gl_ClipDistance[1] = stage_input.gl_ClipDistance0.y;
    gl_CullDistance[0] = stage_input.gl_CullDistance0.x;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
