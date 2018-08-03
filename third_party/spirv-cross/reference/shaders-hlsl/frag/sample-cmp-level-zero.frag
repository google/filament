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
    float s0 = uSampler2D.SampleCmp(_uSampler2D_sampler, vUVRef.xy, vUVRef.z, int2(-1, -1));
    float s1 = uSampler2DArray.SampleCmp(_uSampler2DArray_sampler, vDirRef.xyz, vDirRef.w, int2(-1, -1));
    float s2 = uSamplerCube.SampleCmp(_uSamplerCube_sampler, vDirRef.xyz, vDirRef.w);
    float s3 = uSamplerCubeArray.SampleCmp(_uSamplerCubeArray_sampler, vDirRef, 0.5f);
    float l0 = uSampler2D.SampleCmpLevelZero(_uSampler2D_sampler, vUVRef.xy, vUVRef.z, int2(-1, -1));
    float l1 = uSampler2DArray.SampleCmpLevelZero(_uSampler2DArray_sampler, vDirRef.xyz, vDirRef.w, int2(-1, -1));
    float l2 = uSamplerCube.SampleCmpLevelZero(_uSamplerCube_sampler, vDirRef.xyz, vDirRef.w);
    float4 _80 = vDirRef;
    _80.z = vDirRef.w;
    float p0 = uSampler2D.SampleCmp(_uSampler2D_sampler, SPIRV_Cross_projectTextureCoordinate(_80.xyz), vDirRef.z, int2(1, 1));
    float4 _87 = vDirRef;
    _87.z = vDirRef.w;
    float p1 = uSampler2D.SampleCmpLevelZero(_uSampler2D_sampler, SPIRV_Cross_projectTextureCoordinate(_87.xyz), vDirRef.z, int2(1, 1));
    FragColor = (((((((s0 + s1) + s2) + s3) + l0) + l1) + l2) + p0) + p1;
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
