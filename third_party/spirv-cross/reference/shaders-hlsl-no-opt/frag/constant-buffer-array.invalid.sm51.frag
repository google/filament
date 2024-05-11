struct CBO_1
{
    float4 a;
    float4 b;
    float4 c;
    float4 d;
};

ConstantBuffer<CBO_1> cbo[2][4] : register(b4, space0);
cbuffer PushMe
{
    float4 push_a : packoffset(c0);
    float4 push_b : packoffset(c1);
    float4 push_c : packoffset(c2);
    float4 push_d : packoffset(c3);
};


static float4 FragColor;

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

void frag_main()
{
    FragColor = cbo[1][2].a;
    FragColor += cbo[1][2].b;
    FragColor += cbo[1][2].c;
    FragColor += cbo[1][2].d;
    FragColor += push_a;
    FragColor += push_b;
    FragColor += push_c;
    FragColor += push_d;
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
