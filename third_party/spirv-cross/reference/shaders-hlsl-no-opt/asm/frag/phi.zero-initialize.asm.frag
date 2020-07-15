struct Foo
{
    int a;
};

static float4 vColor;
static float4 FragColor;

struct SPIRV_Cross_Input
{
    float4 vColor : TEXCOORD0;
};

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

static int uninit_int = 0;
static int4 uninit_vector = int4(0, 0, 0, 0);
static float4x4 uninit_matrix = float4x4(0.0f.xxxx, 0.0f.xxxx, 0.0f.xxxx, 0.0f.xxxx);
static Foo uninit_foo = { 0 };

void frag_main()
{
    int _39 = 0;
    if (vColor.x > 10.0f)
    {
        _39 = 10;
    }
    else
    {
        _39 = 20;
    }
    FragColor = vColor;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    vColor = stage_input.vColor;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
