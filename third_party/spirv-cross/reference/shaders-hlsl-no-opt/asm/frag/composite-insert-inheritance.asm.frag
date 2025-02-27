static float4 _33;

static const float4 _35[2] = { 0.0f.xxxx, 0.0f.xxxx };

static float4 vInput;
static float4 FragColor;

struct SPIRV_Cross_Input
{
    float4 vInput : TEXCOORD0;
};

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

void frag_main()
{
    float4 _37 = vInput;
    float4 _38 = _37;
    _38.x = 1.0f;
    _38.y = 2.0f;
    _38.z = 3.0f;
    _38.w = 4.0f;
    FragColor = _38;
    float4 _8 = _37;
    _8.x = 1.0f;
    _8.y = 2.0f;
    _8.z = 3.0f;
    _8.w = 4.0f;
    FragColor = _8;
    float4 _42 = _37;
    _42.x = 1.0f;
    _42.y = 2.0f;
    _42.z = 3.0f;
    _42.w = 4.0f;
    FragColor = _42;
    float4 _44 = _37;
    _44.x = 1.0f;
    float4 _45 = _44;
    _45.y = 2.0f;
    float4 _46 = _45;
    _46.z = 3.0f;
    float4 _47 = _46;
    _47.w = 4.0f;
    FragColor = _47 + _44;
    FragColor = _47 + _45;
    float4 _49;
    _49.x = 1.0f;
    _49.y = 2.0f;
    _49.z = 3.0f;
    _49.w = 4.0f;
    FragColor = _49;
    float4 _53 = 0.0f.xxxx;
    _53.x = 1.0f;
    FragColor = _53;
    float4 _54[2] = _35;
    _54[1].z = 1.0f;
    _54[0].w = 2.0f;
    FragColor = _54[0];
    FragColor = _54[1];
    float4x4 _58 = float4x4(0.0f.xxxx, 0.0f.xxxx, 0.0f.xxxx, 0.0f.xxxx);
    _58[1].z = 1.0f;
    _58[2].w = 2.0f;
    FragColor = _58[0];
    FragColor = _58[1];
    FragColor = _58[2];
    FragColor = _58[3];
    float4 PHI;
    PHI = _46;
    float4 _65 = PHI;
    _65.w = 4.0f;
    FragColor = _65;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    vInput = stage_input.vInput;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
