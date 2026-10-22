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

void func0()
{
#line 104 "test.frag"
#line 106 "test.frag"
    if (iv.x < 0.0f)
    {
#line 107 "test.frag"
        ov.x = 50.0f;
    }
    else
    {
#line 109 "test.frag"
        ov.x = 60.0f;
    }
#line 110 "test.frag"
}

void func1()
{
#line 112 "test.frag"
#line 114 "test.frag"
    for (int i = 0; i < 4; i++)
    {
#line 116 "test.frag"
        func0();
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
#line 122 "test.frag"
}

void func2()
{
#line 124 "test.frag"
#line 126 "test.frag"
    for (int i = 0; i < 4; i++)
    {
#line 128 "test.frag"
        func0();
#line 129 "test.frag"
        func1();
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
#line 135 "test.frag"
}

void frag_main()
{
#line 137 "test.frag"
#line 139 "test.frag"
    func0();
#line 140 "test.frag"
    func1();
#line 141 "test.frag"
    func2();
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
