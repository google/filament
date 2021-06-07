struct UBO_1_1
{
    float4 v[64];
};

ConstantBuffer<UBO_1_1> ubos[] : register(b2, space9);
RWByteAddressBuffer ssbos[] : register(u3, space10);
Texture2D<float4> uSamplers[] : register(t0, space0);
SamplerState uSamps[] : register(s1, space3);
Texture2D<float4> uCombinedSamplers[] : register(t4, space2);
SamplerState _uCombinedSamplers_sampler[] : register(s4, space2);
Texture2DMS<float4> uSamplersMS[] : register(t0, space1);
RWTexture2D<float> uImages[] : register(u5, space7);
RWTexture2D<uint> uImagesU32[] : register(u5, space8);

static int vIndex;
static float4 FragColor;
static float2 vUV;

struct SPIRV_Cross_Input
{
    nointerpolation int vIndex : TEXCOORD0;
    float2 vUV : TEXCOORD1;
};

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

uint2 spvTextureSize(Texture2D<float4> Tex, uint Level, out uint Param)
{
    uint2 ret;
    Tex.GetDimensions(Level, ret.x, ret.y, Param);
    return ret;
}

uint2 spvTextureSize(Texture2DMS<float4> Tex, uint Level, out uint Param)
{
    uint2 ret;
    Tex.GetDimensions(ret.x, ret.y, Param);
    return ret;
}

uint2 spvImageSize(RWTexture2D<float> Tex, out uint Param)
{
    uint2 ret;
    Tex.GetDimensions(ret.x, ret.y);
    Param = 0u;
    return ret;
}

void frag_main()
{
    int _22 = vIndex + 10;
    int _32 = vIndex + 40;
    FragColor = uSamplers[NonUniformResourceIndex(_22)].Sample(uSamps[NonUniformResourceIndex(_32)], vUV);
    int _49 = _22;
    FragColor = uCombinedSamplers[NonUniformResourceIndex(_49)].Sample(_uCombinedSamplers_sampler[NonUniformResourceIndex(_49)], vUV);
    int _65 = vIndex + 20;
    int _69 = _32;
    FragColor += ubos[NonUniformResourceIndex(_65)].v[_69];
    int _83 = vIndex + 50;
    int _88 = vIndex + 60;
    FragColor += asfloat(ssbos[NonUniformResourceIndex(_83)].Load4(_88 * 16 + 16));
    int _100 = vIndex + 70;
    ssbos[NonUniformResourceIndex(_88)].Store4(_100 * 16 + 16, asuint(20.0f.xxxx));
    int2 _111 = int2(vUV);
    FragColor = uSamplers[NonUniformResourceIndex(_49)].Load(int3(_111, 0));
    int _116 = vIndex + 100;
    uint _122;
    ssbos[_116].InterlockedAdd(0, 100u, _122);
    float _136_tmp = uSamplers[NonUniformResourceIndex(_22)].CalculateLevelOfDetail(uSamps[NonUniformResourceIndex(_32)], vUV);
    float2 _136 = _136_tmp.xx;
    float _143_tmp = uCombinedSamplers[NonUniformResourceIndex(_49)].CalculateLevelOfDetail(_uCombinedSamplers_sampler[NonUniformResourceIndex(_49)], vUV);
    float2 _143 = _143_tmp.xx;
    float2 _149 = FragColor.xy + (_136 + _143);
    FragColor = float4(_149.x, _149.y, FragColor.z, FragColor.w);
    int _157;
    spvTextureSize(uSamplers[NonUniformResourceIndex(_65)], 0u, _157);
    FragColor.x += float(int(_157));
    int _174;
    spvTextureSize(uSamplersMS[NonUniformResourceIndex(_65)], 0u, _174);
    FragColor.y += float(int(_174));
    uint _185_dummy_parameter;
    float2 _189 = FragColor.xy + float2(int2(spvTextureSize(uSamplers[NonUniformResourceIndex(_65)], uint(0), _185_dummy_parameter)));
    FragColor = float4(_189.x, _189.y, FragColor.z, FragColor.w);
    FragColor += uImages[NonUniformResourceIndex(_83)][_111].xxxx;
    uint _212_dummy_parameter;
    float2 _216 = FragColor.xy + float2(int2(spvImageSize(uImages[NonUniformResourceIndex(_65)], _212_dummy_parameter)));
    FragColor = float4(_216.x, _216.y, FragColor.z, FragColor.w);
    uImages[NonUniformResourceIndex(_88)][_111] = 50.0f.x;
    uint _242;
    InterlockedAdd(uImagesU32[NonUniformResourceIndex(_100)][_111], 40u, _242);
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    vIndex = stage_input.vIndex;
    vUV = stage_input.vUV;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
