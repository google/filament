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
    int i = vIndex;
    FragColor = uSamplers[NonUniformResourceIndex(i + 10)].Sample(uSamps[NonUniformResourceIndex(i + 40)], vUV);
    int _49 = i + 10;
    FragColor = uCombinedSamplers[NonUniformResourceIndex(_49)].Sample(_uCombinedSamplers_sampler[NonUniformResourceIndex(_49)], vUV);
    int _65 = i + 20;
    int _69 = i + 40;
    FragColor += ubos[NonUniformResourceIndex(_65)].v[_69];
    int _83 = i + 50;
    int _88 = i + 60;
    FragColor += asfloat(ssbos[NonUniformResourceIndex(_83)].Load4(_88 * 16 + 16));
    int _96 = i + 60;
    int _100 = i + 70;
    ssbos[NonUniformResourceIndex(_96)].Store4(_100 * 16 + 16, asuint(20.0f.xxxx));
    int _106 = i + 10;
    FragColor = uSamplers[NonUniformResourceIndex(_106)].Load(int3(int2(vUV), 0));
    int _116 = i + 100;
    uint _122;
    ssbos[_116].InterlockedAdd(0, 100u, _122);
    float _136_tmp = uSamplers[NonUniformResourceIndex(i + 10)].CalculateLevelOfDetail(uSamps[NonUniformResourceIndex(i + 40)], vUV);
    float2 _136 = _136_tmp.xx;
    float2 queried = _136;
    int _139 = i + 10;
    float _143_tmp = uCombinedSamplers[NonUniformResourceIndex(_139)].CalculateLevelOfDetail(_uCombinedSamplers_sampler[NonUniformResourceIndex(_139)], vUV);
    float2 _143 = _143_tmp.xx;
    queried += _143;
    float4 _147 = FragColor;
    float2 _149 = _147.xy + queried;
    FragColor.x = _149.x;
    FragColor.y = _149.y;
    int _157 = i + 20;
    int _160;
    spvTextureSize(uSamplers[NonUniformResourceIndex(_157)], 0u, _160);
    FragColor.x += float(int(_160));
    int _172 = i + 20;
    int _176;
    spvTextureSize(uSamplersMS[NonUniformResourceIndex(_172)], 0u, _176);
    FragColor.y += float(int(_176));
    int _184 = i + 20;
    uint _187_dummy_parameter;
    float4 _189 = FragColor;
    float2 _191 = _189.xy + float2(int2(spvTextureSize(uSamplers[NonUniformResourceIndex(_184)], uint(0), _187_dummy_parameter)));
    FragColor.x = _191.x;
    FragColor.y = _191.y;
    int _202 = i + 50;
    FragColor += uImages[NonUniformResourceIndex(_202)][int2(vUV)].xxxx;
    int _213 = i + 20;
    uint _216_dummy_parameter;
    float4 _218 = FragColor;
    float2 _220 = _218.xy + float2(int2(spvImageSize(uImages[NonUniformResourceIndex(_213)], _216_dummy_parameter)));
    FragColor.x = _220.x;
    FragColor.y = _220.y;
    int _227 = i + 60;
    uImages[NonUniformResourceIndex(_227)][int2(vUV)] = 50.0f.x;
    int _240 = i + 70;
    uint _248;
    InterlockedAdd(uImagesU32[NonUniformResourceIndex(_240)][int2(vUV)], 40u, _248);
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
