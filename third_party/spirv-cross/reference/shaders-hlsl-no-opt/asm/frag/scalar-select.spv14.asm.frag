struct _15
{
    float _m0;
};

static const _15 _25 = { 0.0f };
static const _15 _26 = { 1.0f };
static const float _29[2] = { 0.0f, 1.0f };
static const float _30[2] = { 1.0f, 0.0f };

static float4 FragColor;

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

void spvSelectComposite(out _15 out_value, bool cond, _15 true_val, _15 false_val)
{
    if (cond)
    {
        out_value = true_val;
    }
    else
    {
        out_value = false_val;
    }
}

void spvSelectComposite(out float out_value[2], bool cond, float true_val[2], float false_val[2])
{
    if (cond)
    {
        out_value = true_val;
    }
    else
    {
        out_value = false_val;
    }
}

void frag_main()
{
    FragColor = false ? float4(1.0f, 1.0f, 0.0f, 1.0f) : float4(0.0f, 0.0f, 0.0f, 1.0f);
    FragColor = false ? 1.0f.xxxx : 0.0f.xxxx;
    FragColor = float4(bool4(false, true, false, true).x ? float4(1.0f, 1.0f, 0.0f, 1.0f).x : float4(0.0f, 0.0f, 0.0f, 1.0f).x, bool4(false, true, false, true).y ? float4(1.0f, 1.0f, 0.0f, 1.0f).y : float4(0.0f, 0.0f, 0.0f, 1.0f).y, bool4(false, true, false, true).z ? float4(1.0f, 1.0f, 0.0f, 1.0f).z : float4(0.0f, 0.0f, 0.0f, 1.0f).z, bool4(false, true, false, true).w ? float4(1.0f, 1.0f, 0.0f, 1.0f).w : float4(0.0f, 0.0f, 0.0f, 1.0f).w);
    FragColor = float4(bool4(false, true, false, true));
    _15 _38;
    spvSelectComposite(_38, false, _25, _26);
    _15 _32 = _38;
    float _39[2];
    spvSelectComposite(_39, true, _29, _30);
    float _33[2] = _39;
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
