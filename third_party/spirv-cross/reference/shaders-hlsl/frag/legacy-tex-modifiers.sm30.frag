uniform sampler2D uSampler;

static float4 FragColor;
static float2 vUV;

struct SPIRV_Cross_Input
{
    float2 vUV : TEXCOORD0;
};

struct SPIRV_Cross_Output
{
    float4 FragColor : COLOR0;
};

void frag_main()
{
    float3 _23 = float3(vUV, 5.0f);
    FragColor = tex2Dproj(uSampler, float4(_23.xy, 0.0, _23.z));
    FragColor += tex2Dbias(uSampler, float4(vUV, 0.0, 3.0f));
    FragColor += tex2Dlod(uSampler, float4(vUV, 0.0, 2.0f));
    FragColor += tex2Dgrad(uSampler, vUV, 4.0f.xx, 5.0f.xx);
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    vUV = stage_input.vUV;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = float4(FragColor);
    return stage_output;
}
