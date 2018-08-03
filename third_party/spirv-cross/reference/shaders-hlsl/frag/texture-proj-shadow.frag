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

float SPIRV_Cross_projectTextureCoordinate(float2 coord)
{
    return coord.x / coord.y;
}

float2 SPIRV_Cross_projectTextureCoordinate(float3 coord)
{
    return float2(coord.x, coord.y) / coord.z;
}

float3 SPIRV_Cross_projectTextureCoordinate(float4 coord)
{
    return float3(coord.x, coord.y, coord.z) / coord.w;
}

void frag_main()
{
    float4 _20 = vClip4;
    _20.y = vClip4.w;
    FragColor = uShadow1D.SampleCmp(_uShadow1D_sampler, SPIRV_Cross_projectTextureCoordinate(_20.xy), vClip4.z);
    float4 _30 = vClip4;
    _30.z = vClip4.w;
    FragColor = uShadow2D.SampleCmp(_uShadow2D_sampler, SPIRV_Cross_projectTextureCoordinate(_30.xyz), vClip4.z);
    FragColor = uSampler1D.Sample(_uSampler1D_sampler, SPIRV_Cross_projectTextureCoordinate(vClip2)).x;
    FragColor = uSampler2D.Sample(_uSampler2D_sampler, SPIRV_Cross_projectTextureCoordinate(vClip3)).x;
    FragColor = uSampler3D.Sample(_uSampler3D_sampler, SPIRV_Cross_projectTextureCoordinate(vClip4)).x;
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
