Texture2D<float4> uSampler2D : register(t0);
SamplerComparisonState _uSampler2D_sampler : register(s0);
Texture2DArray<float4> uSampler2DArray : register(t1);
SamplerComparisonState _uSampler2DArray_sampler : register(s1);
TextureCube<float4> uSamplerCube : register(t2);
SamplerComparisonState _uSamplerCube_sampler : register(s2);
TextureCubeArray<float4> uSamplerCubeArray : register(t3);
SamplerComparisonState _uSamplerCubeArray_sampler : register(s3);

static float3 vUVRef;
static float4 vDirRef;
static float FragColor;

struct SPIRV_Cross_Input
{
    float3 vUVRef : TEXCOORD0;
    float4 vDirRef : TEXCOORD1;
};

struct SPIRV_Cross_Output
{
    float FragColor : SV_Target0;
};

void frag_main()
{
    float4 _80 = vDirRef;
    _80.z = vDirRef.w;
    FragColor = (((((((uSampler2D.SampleCmp(_uSampler2D_sampler, vUVRef.xy, vUVRef.z, int2(-1, -1)) + uSampler2DArray.SampleCmp(_uSampler2DArray_sampler, vDirRef.xyz, vDirRef.w, int2(-1, -1))) + uSamplerCube.SampleCmp(_uSamplerCube_sampler, vDirRef.xyz, vDirRef.w)) + uSamplerCubeArray.SampleCmp(_uSamplerCubeArray_sampler, vDirRef, 0.5f)) + uSampler2D.SampleCmpLevelZero(_uSampler2D_sampler, vUVRef.xy, vUVRef.z, int2(-1, -1))) + uSampler2DArray.SampleCmpLevelZero(_uSampler2DArray_sampler, vDirRef.xyz, vDirRef.w, int2(-1, -1))) + uSamplerCube.SampleCmpLevelZero(_uSamplerCube_sampler, vDirRef.xyz, vDirRef.w)) + uSampler2D.SampleCmp(_uSampler2D_sampler, _80.xy / _80.z, vDirRef.z / _80.z, int2(1, 1))) + uSampler2D.SampleCmpLevelZero(_uSampler2D_sampler, _80.xy / _80.z, vDirRef.z / _80.z, int2(1, 1));
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    vUVRef = stage_input.vUVRef;
    vDirRef = stage_input.vDirRef;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
