cbuffer Buff : register(b0)
{
    uint _15_TestVal : packoffset(c0);
};


static float4 fsout_Color;

struct SPIRV_Cross_Output
{
    float4 fsout_Color : SV_Target0;
};

void frag_main()
{
    fsout_Color = 1.0f.xxxx;
    switch (_15_TestVal)
    {
        case 0u:
        {
            fsout_Color = 0.100000001490116119384765625f.xxxx;
            break;
        }
        case 1u:
        {
            fsout_Color = 0.20000000298023223876953125f.xxxx;
            break;
        }
    }
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.fsout_Color = fsout_Color;
    return stage_output;
}
