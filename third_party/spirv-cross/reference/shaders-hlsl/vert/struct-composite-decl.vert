struct VOut
{
    float4 a;
    float4 b;
    float4 c;
    float4 d;
};

static VOut vout;
static float4 a;
static float4 b;
static float4 c;
static float4 d;

struct SPIRV_Cross_Input
{
    float4 a : TEXCOORD0;
    float4 b : TEXCOORD1;
    float4 c : TEXCOORD2;
    float4 d : TEXCOORD3;
};

struct SPIRV_Cross_Output
{
    VOut vout : TEXCOORD0;
};

void emit_result(VOut v)
{
    vout = v;
}

void vert_main()
{
    VOut _26 = { a, b, c, d };
    VOut param = _26;
    emit_result(param);
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    a = stage_input.a;
    b = stage_input.b;
    c = stage_input.c;
    d = stage_input.d;
    vert_main();
    SPIRV_Cross_Output stage_output;
    stage_output.vout = vout;
    return stage_output;
}
