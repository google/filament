Texture1D<uint4> uSampler1DUint : register(t0);
SamplerState _uSampler1DUint_sampler : register(s0);
Texture1D<int4> uSampler1DInt : register(t0);
SamplerState _uSampler1DInt_sampler : register(s0);
Texture1D<float4> uSampler1DFloat : register(t0);
SamplerState _uSampler1DFloat_sampler : register(s0);
Texture2DArray<int4> uSampler2DArray : register(t2);
SamplerState _uSampler2DArray_sampler : register(s2);
Texture3D<float4> uSampler3D : register(t3);
SamplerState _uSampler3D_sampler : register(s3);
TextureCube<float4> uSamplerCube : register(t4);
SamplerState _uSamplerCube_sampler : register(s4);
TextureCubeArray<uint4> uSamplerCubeArray : register(t5);
SamplerState _uSamplerCubeArray_sampler : register(s5);
Buffer<float4> uSamplerBuffer : register(t6);
Texture2DMS<int4> uSamplerMS : register(t7);
SamplerState _uSamplerMS_sampler : register(s7);
Texture2DMSArray<float4> uSamplerMSArray : register(t8);
SamplerState _uSamplerMSArray_sampler : register(s8);
Texture2D<float4> uSampler2D : register(t1);
SamplerState _uSampler2D_sampler : register(s1);

uint SPIRV_Cross_textureSize(Texture1D<float4> Tex, uint Level, out uint Param)
{
    uint ret;
    Tex.GetDimensions(Level, ret.x, Param);
    return ret;
}

uint SPIRV_Cross_textureSize(Texture1D<int4> Tex, uint Level, out uint Param)
{
    uint ret;
    Tex.GetDimensions(Level, ret.x, Param);
    return ret;
}

uint SPIRV_Cross_textureSize(Texture1D<uint4> Tex, uint Level, out uint Param)
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

uint3 SPIRV_Cross_textureSize(Texture2DArray<int4> Tex, uint Level, out uint Param)
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

uint3 SPIRV_Cross_textureSize(TextureCubeArray<uint4> Tex, uint Level, out uint Param)
{
    uint3 ret;
    Tex.GetDimensions(Level, ret.x, ret.y, ret.z, Param);
    return ret;
}

uint2 SPIRV_Cross_textureSize(Texture2DMS<int4> Tex, uint Level, out uint Param)
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
    uint _24_dummy_parameter;
    uint _32_dummy_parameter;
    uint _42_dummy_parameter;
    uint _50_dummy_parameter;
    uint _60_dummy_parameter;
    uint _68_dummy_parameter;
    uint _76_dummy_parameter;
    uint _84_dummy_parameter;
    uint _92_dummy_parameter;
    int _100;
    SPIRV_Cross_textureSize(uSampler2D, 0u, _100);
    int _104;
    SPIRV_Cross_textureSize(uSampler2DArray, 0u, _104);
    int _108;
    SPIRV_Cross_textureSize(uSampler3D, 0u, _108);
    int _112;
    SPIRV_Cross_textureSize(uSamplerCube, 0u, _112);
    int _116;
    SPIRV_Cross_textureSize(uSamplerMS, 0u, _116);
    int _120;
    SPIRV_Cross_textureSize(uSamplerMSArray, 0u, _120);
}

void main()
{
    frag_main();
}
