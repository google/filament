RWByteAddressBuffer _9 : register(u6, space0);
globallycoherent RasterizerOrderedByteAddressBuffer _42 : register(u3, space0);
RasterizerOrderedByteAddressBuffer _52 : register(u4, space0);
RWTexture2D<unorm float4> img4 : register(u5, space0);
RasterizerOrderedTexture2D<unorm float4> img : register(u0, space0);
RasterizerOrderedTexture2D<unorm float4> img3 : register(u2, space0);
RasterizerOrderedTexture2D<uint> img2 : register(u1, space0);

void frag_main()
{
    _9.Store(0, uint(0));
    img4[int2(1, 1)] = float4(1.0f, 0.0f, 0.0f, 1.0f);
    img[int2(0, 0)] = img3[int2(0, 0)];
    uint _39;
    InterlockedAdd(img2[int2(0, 0)], 1u, _39);
    _42.Store(0, uint(int(_42.Load(0)) + 42));
    uint _55;
    _42.InterlockedAnd(4, _52.Load(0), _55);
}

void main()
{
    frag_main();
}
