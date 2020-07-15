RWTexture1D<float4> uImage1D : register(u0);
RWTexture2D<float2> uImage2D : register(u1);
Texture2DArray<float4> uImage2DArray : register(t2);
RWTexture3D<unorm float4> uImage3D : register(u3);
RWBuffer<snorm float4> uImageBuffer : register(u6);

uint3 SPIRV_Cross_textureSize(Texture2DArray<float4> Tex, uint Level, out uint Param)
{
    uint3 ret;
    Tex.GetDimensions(Level, ret.x, ret.y, ret.z, Param);
    return ret;
}

uint2 SPIRV_Cross_imageSize(RWTexture2D<float2> Tex, out uint Param)
{
    uint2 ret;
    Tex.GetDimensions(ret.x, ret.y);
    Param = 0u;
    return ret;
}

uint SPIRV_Cross_imageSize(RWTexture1D<float4> Tex, out uint Param)
{
    uint ret;
    Tex.GetDimensions(ret.x);
    Param = 0u;
    return ret;
}

uint3 SPIRV_Cross_imageSize(RWTexture3D<unorm float4> Tex, out uint Param)
{
    uint3 ret;
    Tex.GetDimensions(ret.x, ret.y, ret.z);
    Param = 0u;
    return ret;
}

uint SPIRV_Cross_imageSize(RWBuffer<snorm float4> Tex, out uint Param)
{
    uint ret;
    Tex.GetDimensions(ret.x);
    Param = 0u;
    return ret;
}

void frag_main()
{
    uint _14_dummy_parameter;
    int a = int(SPIRV_Cross_imageSize(uImage1D, _14_dummy_parameter));
    uint _22_dummy_parameter;
    int2 b = int2(SPIRV_Cross_imageSize(uImage2D, _22_dummy_parameter));
    uint _30_dummy_parameter;
    int3 c = int3(SPIRV_Cross_textureSize(uImage2DArray, 0u, _30_dummy_parameter));
    uint _36_dummy_parameter;
    int3 d = int3(SPIRV_Cross_imageSize(uImage3D, _36_dummy_parameter));
    uint _42_dummy_parameter;
    int e = int(SPIRV_Cross_imageSize(uImageBuffer, _42_dummy_parameter));
}

void main()
{
    frag_main();
}
