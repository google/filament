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
    float4 t = uSampler.Sample(_uSampler_sampler, vInput.xy);
    float4 d0 = ddx(vInput);
    float4 d1 = ddy(vInput);
    float4 d2 = fwidth(vInput);
    float4 d3 = ddx_coarse(vInput);
    float4 d4 = ddy_coarse(vInput);
    float4 d5 = fwidth(vInput);
    float4 d6 = ddx_fine(vInput);
    float4 d7 = ddy_fine(vInput);
    float4 d8 = fwidth(vInput);
    float _56_tmp = uSampler.CalculateLevelOfDetail(_uSampler_sampler, vInput.zw);
    float2 lod = float2(_56_tmp, _56_tmp);
    if (vInput.y > 10.0f)
    {
        FragColor += t;
        FragColor += d0;
        FragColor += d1;
        FragColor += d2;
        FragColor += d3;
        FragColor += d4;
        FragColor += d5;
        FragColor += d6;
        FragColor += d7;
        FragColor += d8;
        FragColor += lod.xyxy;
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
