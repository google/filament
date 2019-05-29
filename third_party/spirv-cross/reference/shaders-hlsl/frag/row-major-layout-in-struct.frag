struct NonFoo
{
    float4x4 v;
    float4x4 w;
};

struct Foo
{
    row_major float4x4 v;
    row_major float4x4 w;
};

cbuffer UBO : register(b0)
{
    Foo _17_foo : packoffset(c0);
};


static float4 FragColor;
static float4 vUV;

struct SPIRV_Cross_Input
{
    float4 vUV : TEXCOORD0;
};

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

void frag_main()
{
    NonFoo f;
    f.v = _17_foo.v;
    f.w = _17_foo.w;
    FragColor = mul(mul(vUV, f.w), f.v);
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    vUV = stage_input.vUV;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
