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

uint spvTextureSize(Texture1D<float4> Tex, uint Level, out uint Param)
{
    uint ret;
    Tex.GetDimensions(Level, ret.x, Param);
    return ret;
}

uint spvTextureSize(Texture1D<int4> Tex, uint Level, out uint Param)
{
    uint ret;
    Tex.GetDimensions(Level, ret.x, Param);
    return ret;
}

uint spvTextureSize(Texture1D<uint4> Tex, uint Level, out uint Param)
{
    uint ret;
    Tex.GetDimensions(Level, ret.x, Param);
    return ret;
}

uint2 spvTextureSize(Texture2D<float4> Tex, uint Level, out uint Param)
{
    uint2 ret;
    Tex.GetDimensions(Level, ret.x, ret.y, Param);
    return ret;
}

uint3 spvTextureSize(Texture2DArray<int4> Tex, uint Level, out uint Param)
{
    uint3 ret;
    Tex.GetDimensions(Level, ret.x, ret.y, ret.z, Param);
    return ret;
}

uint3 spvTextureSize(Texture3D<float4> Tex, uint Level, out uint Param)
{
    uint3 ret;
    Tex.GetDimensions(Level, ret.x, ret.y, ret.z, Param);
    return ret;
}

uint spvTextureSize(Buffer<float4> Tex, uint Level, out uint Param)
{
    uint ret;
    Tex.GetDimensions(ret.x);
    Param = 0u;
    return ret;
}

uint2 spvTextureSize(TextureCube<float4> Tex, uint Level, out uint Param)
{
    uint2 ret;
    Tex.GetDimensions(Level, ret.x, ret.y, Param);
    return ret;
}

uint3 spvTextureSize(TextureCubeArray<uint4> Tex, uint Level, out uint Param)
{
    uint3 ret;
    Tex.GetDimensions(Level, ret.x, ret.y, ret.z, Param);
    return ret;
}

uint2 spvTextureSize(Texture2DMS<int4> Tex, uint Level, out uint Param)
{
    uint2 ret;
    Tex.GetDimensions(ret.x, ret.y, Param);
    return ret;
}

uint3 spvTextureSize(Texture2DMSArray<float4> Tex, uint Level, out uint Param)
{
    uint3 ret;
    Tex.GetDimensions(ret.x, ret.y, ret.z, Param);
    return ret;
}

void frag_main()
{
    uint _17_dummy_parameter;
    int a = int(spvTextureSize(uSampler1DUint, uint(0), _17_dummy_parameter));
    uint _24_dummy_parameter;
    a = int(spvTextureSize(uSampler1DInt, uint(0), _24_dummy_parameter));
    uint _32_dummy_parameter;
    a = int(spvTextureSize(uSampler1DFloat, uint(0), _32_dummy_parameter));
    uint _42_dummy_parameter;
    int3 c = int3(spvTextureSize(uSampler2DArray, uint(0), _42_dummy_parameter));
    uint _50_dummy_parameter;
    int3 d = int3(spvTextureSize(uSampler3D, uint(0), _50_dummy_parameter));
    uint _60_dummy_parameter;
    int2 e = int2(spvTextureSize(uSamplerCube, uint(0), _60_dummy_parameter));
    uint _68_dummy_parameter;
    int3 f = int3(spvTextureSize(uSamplerCubeArray, uint(0), _68_dummy_parameter));
    uint _76_dummy_parameter;
    int g = int(spvTextureSize(uSamplerBuffer, 0u, _76_dummy_parameter));
    uint _84_dummy_parameter;
    int2 h = int2(spvTextureSize(uSamplerMS, 0u, _84_dummy_parameter));
    uint _92_dummy_parameter;
    int3 i = int3(spvTextureSize(uSamplerMSArray, 0u, _92_dummy_parameter));
    int _100;
    spvTextureSize(uSampler2D, 0u, _100);
    int l1 = int(_100);
    int _104;
    spvTextureSize(uSampler2DArray, 0u, _104);
    int l2 = int(_104);
    int _108;
    spvTextureSize(uSampler3D, 0u, _108);
    int l3 = int(_108);
    int _112;
    spvTextureSize(uSamplerCube, 0u, _112);
    int l4 = int(_112);
    int _116;
    spvTextureSize(uSamplerMS, 0u, _116);
    int s0 = int(_116);
    int _120;
    spvTextureSize(uSamplerMSArray, 0u, _120);
    int s1 = int(_120);
}

void main()
{
    frag_main();
}
