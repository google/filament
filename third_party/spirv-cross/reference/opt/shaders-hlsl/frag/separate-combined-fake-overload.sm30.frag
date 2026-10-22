uniform sampler2D uSamp;
uniform sampler2D SPIRV_Cross_CombineduTuS;

static float4 FragColor;

struct SPIRV_Cross_Output
{
    float4 FragColor : COLOR0;
};

void frag_main()
{
    FragColor = tex2D(uSamp, 0.5f.xx) + tex2D(SPIRV_Cross_CombineduTuS, 0.5f.xx);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = float4(FragColor);
    return stage_output;
}
