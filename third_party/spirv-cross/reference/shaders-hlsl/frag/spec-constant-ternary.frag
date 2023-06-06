#ifndef SPIRV_CROSS_CONSTANT_ID_0
#define SPIRV_CROSS_CONSTANT_ID_0 10u
#endif
static const uint s = SPIRV_CROSS_CONSTANT_ID_0;
static const bool _13 = (s > 20u);
static const uint f = _13 ? 30u : 50u;

static float FragColor;

struct SPIRV_Cross_Output
{
    float FragColor : SV_Target0;
};

void frag_main()
{
    FragColor = float(f);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
