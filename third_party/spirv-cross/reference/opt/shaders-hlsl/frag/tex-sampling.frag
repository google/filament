Texture1D<float4> tex1d : register(t0);
SamplerState _tex1d_sampler : register(s0);
Texture2D<float4> tex2d : register(t1);
SamplerState _tex2d_sampler : register(s1);
Texture3D<float4> tex3d : register(t2);
SamplerState _tex3d_sampler : register(s2);
TextureCube<float4> texCube : register(t3);
SamplerState _texCube_sampler : register(s3);
Texture1D<float4> tex1dShadow : register(t4);
SamplerComparisonState _tex1dShadow_sampler : register(s4);
Texture2D<float4> tex2dShadow : register(t5);
SamplerComparisonState _tex2dShadow_sampler : register(s5);
TextureCube<float4> texCubeShadow : register(t6);
SamplerComparisonState _texCubeShadow_sampler : register(s6);
Texture1DArray<float4> tex1dArray : register(t7);
SamplerState _tex1dArray_sampler : register(s7);
Texture2DArray<float4> tex2dArray : register(t8);
SamplerState _tex2dArray_sampler : register(s8);
TextureCubeArray<float4> texCubeArray : register(t9);
SamplerState _texCubeArray_sampler : register(s9);
Texture2D<float4> separateTex2d : register(t12);
SamplerState samplerNonDepth : register(s11);
Texture2D<float4> separateTex2dDepth : register(t13);
SamplerComparisonState samplerDepth : register(s10);

static float texCoord1d;
static float2 texCoord2d;
static float3 texCoord3d;
static float4 texCoord4d;
static float4 FragColor;

struct SPIRV_Cross_Input
{
    float texCoord1d : TEXCOORD0;
    float2 texCoord2d : TEXCOORD1;
    float3 texCoord3d : TEXCOORD2;
    float4 texCoord4d : TEXCOORD3;
};

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

void frag_main()
{
    float2 _41 = float2(texCoord1d, 2.0f);
    float3 _88 = float3(texCoord2d, 2.0f);
    float4 _135 = float4(texCoord3d, 2.0f);
    float4 _162 = (((((((((((((((((((tex1d.Sample(_tex1d_sampler, texCoord1d) + tex1d.Sample(_tex1d_sampler, texCoord1d, 1)) + tex1d.SampleLevel(_tex1d_sampler, texCoord1d, 2.0f)) + tex1d.SampleGrad(_tex1d_sampler, texCoord1d, 1.0f, 2.0f)) + tex1d.Sample(_tex1d_sampler, _41.x / _41.y)) + tex1d.SampleBias(_tex1d_sampler, texCoord1d, 1.0f)) + tex2d.Sample(_tex2d_sampler, texCoord2d)) + tex2d.Sample(_tex2d_sampler, texCoord2d, int2(1, 2))) + tex2d.SampleLevel(_tex2d_sampler, texCoord2d, 2.0f)) + tex2d.SampleGrad(_tex2d_sampler, texCoord2d, float2(1.0f, 2.0f), float2(3.0f, 4.0f))) + tex2d.Sample(_tex2d_sampler, _88.xy / _88.z)) + tex2d.SampleBias(_tex2d_sampler, texCoord2d, 1.0f)) + tex3d.Sample(_tex3d_sampler, texCoord3d)) + tex3d.Sample(_tex3d_sampler, texCoord3d, int3(1, 2, 3))) + tex3d.SampleLevel(_tex3d_sampler, texCoord3d, 2.0f)) + tex3d.SampleGrad(_tex3d_sampler, texCoord3d, float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f))) + tex3d.Sample(_tex3d_sampler, _135.xyz / _135.w)) + tex3d.SampleBias(_tex3d_sampler, texCoord3d, 1.0f)) + texCube.Sample(_texCube_sampler, texCoord3d)) + texCube.SampleLevel(_texCube_sampler, texCoord3d, 2.0f)) + texCube.SampleBias(_texCube_sampler, texCoord3d, 1.0f);
    _162.w = ((_162.w + tex1dShadow.SampleCmp(_tex1dShadow_sampler, float3(texCoord1d, 0.0f, 0.0f).x, 0.0f)) + tex2dShadow.SampleCmp(_tex2dShadow_sampler, float3(texCoord2d, 0.0f).xy, 0.0f)) + texCubeShadow.SampleCmp(_texCubeShadow_sampler, float4(texCoord3d, 0.0f).xyz, 0.0f);
    float4 _243 = tex2d.GatherRed(_tex2d_sampler, texCoord2d);
    float4 _269 = tex2d.GatherRed(_tex2d_sampler, texCoord2d, int2(1, 1));
    float4 _308 = ((((((((((((((_162 + tex1dArray.Sample(_tex1dArray_sampler, texCoord2d)) + tex2dArray.Sample(_tex2dArray_sampler, texCoord3d)) + texCubeArray.Sample(_texCubeArray_sampler, texCoord4d)) + _243) + _243) + tex2d.GatherGreen(_tex2d_sampler, texCoord2d)) + tex2d.GatherBlue(_tex2d_sampler, texCoord2d)) + tex2d.GatherAlpha(_tex2d_sampler, texCoord2d)) + _269) + _269) + tex2d.GatherGreen(_tex2d_sampler, texCoord2d, int2(1, 1))) + tex2d.GatherBlue(_tex2d_sampler, texCoord2d, int2(1, 1))) + tex2d.GatherAlpha(_tex2d_sampler, texCoord2d, int2(1, 1))) + tex2d.Load(int3(int2(1, 2), 0))) + separateTex2d.Sample(samplerNonDepth, texCoord2d);
    _308.w = _308.w + separateTex2dDepth.SampleCmp(samplerDepth, texCoord3d.xy, texCoord3d.z);
    FragColor = _308;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    texCoord1d = stage_input.texCoord1d;
    texCoord2d = stage_input.texCoord2d;
    texCoord3d = stage_input.texCoord3d;
    texCoord4d = stage_input.texCoord4d;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
