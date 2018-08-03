Texture2D<float4> uTexture : register(t0);
SamplerState _uTexture_sampler : register(s0);

static int2 Size;

struct SPIRV_Cross_Output
{
    int2 Size : SV_Target0;
};

uint2 SPIRV_Cross_textureSize(Texture2D<float4> Tex, uint Level, out uint Param)
{
    uint2 ret;
    Tex.GetDimensions(Level, ret.x, ret.y, Param);
    return ret;
}

void frag_main()
{
    uint _19_dummy_parameter;
    uint _20_dummy_parameter;
    Size = int2(SPIRV_Cross_textureSize(uTexture, uint(0), _19_dummy_parameter)) + int2(SPIRV_Cross_textureSize(uTexture, uint(1), _20_dummy_parameter));
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.Size = Size;
    return stage_output;
}
