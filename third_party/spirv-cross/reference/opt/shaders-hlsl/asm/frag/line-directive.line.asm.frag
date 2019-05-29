static float FragColor;
static float vColor;

struct SPIRV_Cross_Input
{
    float vColor : TEXCOORD0;
};

struct SPIRV_Cross_Output
{
    float FragColor : SV_Target0;
};

#line 8 "test.frag"
void frag_main()
{
    float _80;
#line 8 "test.frag"
    FragColor = 1.0f;
#line 9 "test.frag"
    FragColor = 2.0f;
#line 10 "test.frag"
    _80 = vColor;
    if (_80 < 0.0f)
    {
#line 12 "test.frag"
        FragColor = 3.0f;
    }
    else
    {
#line 16 "test.frag"
        FragColor = 4.0f;
    }
    for (int _126 = 0; float(_126) < (40.0f + _80); )
    {
#line 21 "test.frag"
        FragColor += 0.20000000298023223876953125f;
#line 22 "test.frag"
        FragColor += 0.300000011920928955078125f;
        _126 += (int(_80) + 5);
        continue;
    }
    switch (int(_80))
    {
        case 0:
        {
#line 28 "test.frag"
            FragColor += 0.20000000298023223876953125f;
#line 29 "test.frag"
            break;
        }
        case 1:
        {
#line 32 "test.frag"
            FragColor += 0.4000000059604644775390625f;
#line 33 "test.frag"
            break;
        }
        default:
        {
#line 36 "test.frag"
            FragColor += 0.800000011920928955078125f;
#line 37 "test.frag"
            break;
        }
    }
    for (;;)
    {
        FragColor += (10.0f + _80);
#line 43 "test.frag"
        if (FragColor < 100.0f)
        {
        }
        else
        {
            break;
        }
    }
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    vColor = stage_input.vColor;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
