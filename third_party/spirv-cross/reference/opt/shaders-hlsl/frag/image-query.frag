Texture1D<float4> uSampler1D : register(t0);
SamplerState _uSampler1D_sampler : register(s0);
Texture2D<float4> uSampler2D : register(t1);
SamplerState _uSampler2D_sampler : register(s1);
Texture2DArray<float4> uSampler2DArray : register(t2);
SamplerState _uSampler2DArray_sampler : register(s2);
Texture3D<float4> uSampler3D : register(t3);
SamplerState _uSampler3D_sampler : register(s3);
TextureCube<float4> uSamplerCube : register(t4);
SamplerState _uSamplerCube_sampler : register(s4);
TextureCubeArray<float4> uSamplerCubeArray : register(t5);
SamplerState _uSamplerCubeArray_sampler : register(s5);
Buffer<float4> uSamplerBuffer : register(t6);
Texture2DMS<float4> uSamplerMS : register(t7);
SamplerState _uSamplerMS_sampler : register(s7);
Texture2DMSArray<float4> uSamplerMSArray : register(t8);
SamplerState _uSamplerMSArray_sampler : register(s8);

uint SPIRV_Cross_textureSize(Texture1D<float4> Tex, uint Level, out uint Param)
{
    uint ret;
    Tex.GetDimensions(Level, ret.x, Param);
    return ret;
}

uint2 SPIRV_Cross_textureSize(Texture2D<float4> Tex, uint Level, out uint Param)
{
    uint2 ret;
    Tex.GetDimensions(Level, ret.x, ret.y, Param);
    return ret;
}

uint3 SPIRV_Cross_textureSize(Texture2DArray<float4> Tex, uint Level, out uint Param)
{
    uint3 ret;
    Tex.GetDimensions(Level, ret.x, ret.y, ret.z, Param);
    return ret;
}

uint3 SPIRV_Cross_textureSize(Texture3D<float4> Tex, uint Level, out uint Param)
{
    uint3 ret;
    Tex.GetDimensions(Level, ret.x, ret.y, ret.z, Param);
    return ret;
}

uint SPIRV_Cross_textureSize(Buffer<float4> Tex, uint Level, out uint Param)
{
    uint ret;
    Tex.GetDimensions(ret.x);
    Param = 0u;
    return ret;
}

uint2 SPIRV_Cross_textureSize(TextureCube<float4> Tex, uint Level, out uint Param)
{
    uint2 ret;
    Tex.GetDimensions(Level, ret.x, ret.y, Param);
    return ret;
}

uint3 SPIRV_Cross_textureSize(TextureCubeArray<float4> Tex, uint Level, out uint Param)
{
    uint3 ret;
    Tex.GetDimensions(Level, ret.x, ret.y, ret.z, Param);
    return ret;
}

uint2 SPIRV_Cross_textureSize(Texture2DMS<float4> Tex, uint Level, out uint Param)
{
    uint2 ret;
    Tex.GetDimensions(ret.x, ret.y, Param);
    return ret;
}

uint3 SPIRV_Cross_textureSize(Texture2DMSArray<float4> Tex, uint Level, out uint Param)
{
    uint3 ret;
    Tex.GetDimensions(ret.x, ret.y, ret.z, Param);
    return ret;
}

void frag_main()
{
    uint _17_dummy_parameter;
    uint _27_dummy_parameter;
    uint _37_dummy_parameter;
    uint _45_dummy_parameter;
    uint _53_dummy_parameter;
    uint _61_dummy_parameter;
    uint _69_dummy_parameter;
    uint _77_dummy_parameter;
    uint _85_dummy_parameter;
    int _89;
    SPIRV_Cross_textureSize(uSampler1D, 0u, _89);
    int _93;
    SPIRV_Cross_textureSize(uSampler2D, 0u, _93);
    int _97;
    SPIRV_Cross_textureSize(uSampler2DArray, 0u, _97);
    int _101;
    SPIRV_Cross_textureSize(uSampler3D, 0u, _101);
    int _105;
    SPIRV_Cross_textureSize(uSamplerCube, 0u, _105);
    int _109;
    SPIRV_Cross_textureSize(uSamplerCubeArray, 0u, _109);
    int _113;
    SPIRV_Cross_textureSize(uSamplerMS, 0u, _113);
    int _117;
    SPIRV_Cross_textureSize(uSamplerMSArray, 0u, _117);
}

void main()
{
    frag_main();
}
