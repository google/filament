static float4 FragColor;

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

float4 foo(float4 foo_1)
{
    return foo_1 + 1.0f.xxxx;
}

float4 foo(float3 foo_1)
{
    return foo_1.xyzz + 1.0f.xxxx;
}

float4 foo_1(float4 foo_2)
{
    return foo_2 + 2.0f.xxxx;
}

float4 foo(float2 foo_2)
{
    return foo_2.xyxy + 2.0f.xxxx;
}

void frag_main()
{
    float4 foo_3 = 1.0f.xxxx;
    float4 foo_2 = foo(foo_3);
    float3 foo_5 = 1.0f.xxx;
    float4 foo_4 = foo(foo_5);
    float4 foo_7 = 1.0f.xxxx;
    float4 foo_6 = foo_1(foo_7);
    float2 foo_9 = 1.0f.xx;
    float4 foo_8 = foo(foo_9);
    FragColor = ((foo_2 + foo_4) + foo_6) + foo_8;
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
