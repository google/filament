Texture1D<float4> uShadow1D : register(t0);
SamplerComparisonState _uShadow1D_sampler : register(s0);
Texture2D<float4> uShadow2D : register(t1);
SamplerComparisonState _uShadow2D_sampler : register(s1);
Texture1D<float4> uSampler1D : register(t2);
SamplerState _uSampler1D_sampler : register(s2);
Texture2D<float4> uSampler2D : register(t3);
SamplerState _uSampler2D_sampler : register(s3);
Texture3D<float4> uSampler3D : register(t4);
SamplerState _uSampler3D_sampler : register(s4);

static float FragColor;
static float4 vClip4;
static float2 vClip2;
static float3 vClip3;

struct SPIRV_Cross_Input
{
    float3 vClip3 : TEXCOORD0;
    float4 vClip4 : TEXCOORD1;
    float2 vClip2 : TEXCOORD2;
};

struct SPIRV_Cross_Output
{
    float FragColor : SV_Target0;
};

void frag_main()
{
    float4 _17 = vClip4;
    float4 _20 = _17;
    _20.y = _17.w;
    FragColor = uShadow1D.SampleCmp(_uShadow1D_sampler, _20.x / _20.y, _17.z / _20.y);
    float4 _27 = vClip4;
    float4 _30 = _27;
    _30.z = _27.w;
    FragColor = uShadow2D.SampleCmp(_uShadow2D_sampler, _30.xy / _30.z, _27.z / _30.z);
    FragColor = uSampler1D.Sample(_uSampler1D_sampler, vClip2.x / vClip2.y).x;
    FragColor = uSampler2D.Sample(_uSampler2D_sampler, vClip3.xy / vClip3.z).x;
    FragColor = uSampler3D.Sample(_uSampler3D_sampler, vClip4.xyz / vClip4.w).x;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    vClip4 = stage_input.vClip4;
    vClip2 = stage_input.vClip2;
    vClip3 = stage_input.vClip3;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
