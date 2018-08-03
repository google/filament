Texture2D<float4> uTex : register(t0);
SamplerState uSampler : register(s1);

static int2 FooOut;

struct SPIRV_Cross_Output
{
    int2 FooOut : SV_Target0;
};

uint2 SPIRV_Cross_textureSize(Texture2D<float4> Tex, uint Level, out uint Param)
{
    uint2 ret;
    Tex.GetDimensions(Level, ret.x, ret.y, Param);
    return ret;
}

void frag_main()
{
    uint _23_dummy_parameter;
    FooOut = int2(SPIRV_Cross_textureSize(uTex, uint(0), _23_dummy_parameter));
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FooOut = FooOut;
    return stage_output;
}
