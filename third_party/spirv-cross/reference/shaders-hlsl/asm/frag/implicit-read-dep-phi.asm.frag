Texture2D<float4> uImage : register(t0);
SamplerState _uImage_sampler : register(s0);

static float4 v0;
static float4 FragColor;

struct SPIRV_Cross_Input
{
    float4 v0 : TEXCOORD0;
};

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

void frag_main()
{
    int i = 0;
    float phi;
    float4 _45;
    phi = 1.0f;
    _45 = float4(1.0f, 2.0f, 1.0f, 2.0f);
    for (;;)
    {
        FragColor = _45;
        if (i < 4)
        {
            if (v0[i] > 0.0f)
            {
                float2 _43 = phi.xx;
                i++;
                phi += 2.0f;
                _45 = uImage.SampleLevel(_uImage_sampler, _43, 0.0f);
                continue;
            }
            else
            {
                break;
            }
        }
        else
        {
            break;
        }
    }
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    v0 = stage_input.v0;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
