// REQUIRES: spirv
// RUN: %dxc -T ps_6_0 -E main %s -spirv -fcgl -verify

struct Struct { float f; };

vk::SampledTexture2D<Struct> t; // expected-error {{elements of typed buffers and textures must be scalars or vectors}}

float4 main(float2 f2 : F2) : SV_TARGET
{
    return t.Sample(f2);
}
