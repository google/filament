static float3 iv;
static float3 ov;

struct SPIRV_Cross_Input
{
    float3 iv : TEXCOORD0;
};

struct SPIRV_Cross_Output
{
    float3 ov : SV_Target0;
};

void frag_main()
{
#line 137 "test.frag"
#line 106 "test.frag"
    bool _288 = iv.x < 0.0f;
    if (_288)
    {
#line 107 "test.frag"
        ov.x = 50.0f;
    }
    else
    {
#line 109 "test.frag"
        ov.x = 60.0f;
    }
#line 114 "test.frag"
    for (int _519 = 0; _519 < 4; _519++)
    {
#line 106 "test.frag"
        if (_288)
        {
#line 107 "test.frag"
            ov.x = 50.0f;
        }
        else
        {
#line 109 "test.frag"
            ov.x = 60.0f;
        }
#line 117 "test.frag"
        if (iv.y < 0.0f)
        {
#line 118 "test.frag"
            ov.y = 70.0f;
        }
        else
        {
#line 120 "test.frag"
            ov.y = 80.0f;
        }
    }
#line 126 "test.frag"
    for (int _523 = 0; _523 < 4; _523++)
    {
#line 106 "test.frag"
        if (_288)
        {
#line 107 "test.frag"
            ov.x = 50.0f;
        }
        else
        {
#line 109 "test.frag"
            ov.x = 60.0f;
        }
#line 114 "test.frag"
        for (int _527 = 0; _527 < 4; _527++)
        {
#line 106 "test.frag"
            if (_288)
            {
#line 107 "test.frag"
                ov.x = 50.0f;
            }
            else
            {
#line 109 "test.frag"
                ov.x = 60.0f;
            }
#line 117 "test.frag"
            if (iv.y < 0.0f)
            {
#line 118 "test.frag"
                ov.y = 70.0f;
            }
            else
            {
#line 120 "test.frag"
                ov.y = 80.0f;
            }
        }
#line 130 "test.frag"
        if (iv.z < 0.0f)
        {
#line 131 "test.frag"
            ov.z = 100.0f;
        }
        else
        {
#line 133 "test.frag"
            ov.z = 120.0f;
        }
    }
#line 142 "test.frag"
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    iv = stage_input.iv;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.ov = ov;
    return stage_output;
}
