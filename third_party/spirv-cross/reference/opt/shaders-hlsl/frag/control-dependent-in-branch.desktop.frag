Texture2D<float4> uSampler : register(t0);
SamplerState _uSampler_sampler : register(s0);

static float4 FragColor;
static float4 vInput;

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
    FragColor = vInput;
    float4 _23 = uSampler.Sample(_uSampler_sampler, vInput.xy);
    float4 _26 = ddx(vInput);
    float4 _29 = ddy(vInput);
    float4 _32 = fwidth(vInput);
    float4 _35 = ddx_coarse(vInput);
    float4 _38 = ddy_coarse(vInput);
    float4 _41 = fwidth(vInput);
    float4 _44 = ddx_fine(vInput);
    float4 _47 = ddy_fine(vInput);
    float4 _50 = fwidth(vInput);
    float _56_tmp = uSampler.CalculateLevelOfDetail(_uSampler_sampler, vInput.zw);
    if (vInput.y > 10.0f)
    {
        FragColor += _23;
        FragColor += _26;
        FragColor += _29;
        FragColor += _32;
        FragColor += _35;
        FragColor += _38;
        FragColor += _41;
        FragColor += _44;
        FragColor += _47;
        FragColor += _50;
        FragColor += float2(_56_tmp, _56_tmp).xyxy;
    }
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    vInput = stage_input.vInput;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
