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

uint spvTextureSize(Texture1D<float4> Tex, uint Level, out uint Param)
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

uint3 spvTextureSize(Texture2DArray<float4> Tex, uint Level, out uint Param)
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

uint3 spvTextureSize(TextureCubeArray<float4> Tex, uint Level, out uint Param)
{
    uint3 ret;
    Tex.GetDimensions(Level, ret.x, ret.y, ret.z, Param);
    return ret;
}

uint2 spvTextureSize(Texture2DMS<float4> Tex, uint Level, out uint Param)
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
    int a = int(spvTextureSize(uSampler1D, uint(0), _17_dummy_parameter));
    uint _27_dummy_parameter;
    int2 b = int2(spvTextureSize(uSampler2D, uint(0), _27_dummy_parameter));
    uint _37_dummy_parameter;
    int3 c = int3(spvTextureSize(uSampler2DArray, uint(0), _37_dummy_parameter));
    uint _45_dummy_parameter;
    int3 d = int3(spvTextureSize(uSampler3D, uint(0), _45_dummy_parameter));
    uint _53_dummy_parameter;
    int2 e = int2(spvTextureSize(uSamplerCube, uint(0), _53_dummy_parameter));
    uint _61_dummy_parameter;
    int3 f = int3(spvTextureSize(uSamplerCubeArray, uint(0), _61_dummy_parameter));
    uint _69_dummy_parameter;
    int g = int(spvTextureSize(uSamplerBuffer, 0u, _69_dummy_parameter));
    uint _77_dummy_parameter;
    int2 h = int2(spvTextureSize(uSamplerMS, 0u, _77_dummy_parameter));
    uint _85_dummy_parameter;
    int3 i = int3(spvTextureSize(uSamplerMSArray, 0u, _85_dummy_parameter));
    int _89;
    spvTextureSize(uSampler1D, 0u, _89);
    int l0 = int(_89);
    int _93;
    spvTextureSize(uSampler2D, 0u, _93);
    int l1 = int(_93);
    int _97;
    spvTextureSize(uSampler2DArray, 0u, _97);
    int l2 = int(_97);
    int _101;
    spvTextureSize(uSampler3D, 0u, _101);
    int l3 = int(_101);
    int _105;
    spvTextureSize(uSamplerCube, 0u, _105);
    int l4 = int(_105);
    int _109;
    spvTextureSize(uSamplerCubeArray, 0u, _109);
    int l5 = int(_109);
    int _113;
    spvTextureSize(uSamplerMS, 0u, _113);
    int s0 = int(_113);
    int _117;
    spvTextureSize(uSamplerMSArray, 0u, _117);
    int s1 = int(_117);
}

void main()
{
    frag_main();
}
