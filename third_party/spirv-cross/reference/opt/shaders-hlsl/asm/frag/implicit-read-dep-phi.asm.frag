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
    float phi;
    float4 _36;
    int _51;
    _51 = 0;
    phi = 1.0f;
    _36 = float4(1.0f, 2.0f, 1.0f, 2.0f);
    for (;;)
    {
        FragColor = _36;
        if (_51 < 4)
        {
            if (v0[_51] > 0.0f)
            {
                float2 _48 = phi.xx;
                _51++;
                phi += 2.0f;
                _36 = uImage.SampleLevel(_uImage_sampler, _48, 0.0f);
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
