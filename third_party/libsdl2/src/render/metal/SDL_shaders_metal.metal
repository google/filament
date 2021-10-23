#include <metal_texture>
#include <metal_matrix>

using namespace metal;

struct SolidVertexInput
{
    float2 position [[attribute(0)]];
};

struct SolidVertexOutput
{
    float4 position [[position]];
    float pointSize [[point_size]];
};

vertex SolidVertexOutput SDL_Solid_vertex(SolidVertexInput in [[stage_in]],
                                          constant float4x4 &projection [[buffer(2)]],
                                          constant float4x4 &transform [[buffer(3)]])
{
    SolidVertexOutput v;
    v.position = (projection * transform) * float4(in.position, 0.0f, 1.0f);
    v.pointSize = 1.0f;
    return v;
}

fragment float4 SDL_Solid_fragment(const device float4 &col [[buffer(0)]])
{
    return col;
}

struct CopyVertexInput
{
    float2 position [[attribute(0)]];
    float2 texcoord [[attribute(1)]];
};

struct CopyVertexOutput
{
    float4 position [[position]];
    float2 texcoord;
};

vertex CopyVertexOutput SDL_Copy_vertex(CopyVertexInput in [[stage_in]],
                                        constant float4x4 &projection [[buffer(2)]],
                                        constant float4x4 &transform [[buffer(3)]])
{
    CopyVertexOutput v;
    v.position = (projection * transform) * float4(in.position, 0.0f, 1.0f);
    v.texcoord = in.texcoord;
    return v;
}

fragment float4 SDL_Copy_fragment(CopyVertexOutput vert [[stage_in]],
                                  const device float4 &col [[buffer(0)]],
                                  texture2d<float> tex [[texture(0)]],
                                  sampler s [[sampler(0)]])
{
    return tex.sample(s, vert.texcoord) * col;
}

struct YUVDecode
{
    float3 offset;
    float3 Rcoeff;
    float3 Gcoeff;
    float3 Bcoeff;
};

fragment float4 SDL_YUV_fragment(CopyVertexOutput vert [[stage_in]],
                                 const device float4 &col [[buffer(0)]],
                                 constant YUVDecode &decode [[buffer(1)]],
                                 texture2d<float> texY [[texture(0)]],
                                 texture2d_array<float> texUV [[texture(1)]],
                                 sampler s [[sampler(0)]])
{
    float3 yuv;
    yuv.x = texY.sample(s, vert.texcoord).r;
    yuv.y = texUV.sample(s, vert.texcoord, 0).r;
    yuv.z = texUV.sample(s, vert.texcoord, 1).r;

    yuv += decode.offset;

    return col * float4(dot(yuv, decode.Rcoeff), dot(yuv, decode.Gcoeff), dot(yuv, decode.Bcoeff), 1.0);
}

fragment float4 SDL_NV12_fragment(CopyVertexOutput vert [[stage_in]],
                                 const device float4 &col [[buffer(0)]],
                                 constant YUVDecode &decode [[buffer(1)]],
                                 texture2d<float> texY [[texture(0)]],
                                 texture2d<float> texUV [[texture(1)]],
                                 sampler s [[sampler(0)]])
{
    float3 yuv;
    yuv.x = texY.sample(s, vert.texcoord).r;
    yuv.yz = texUV.sample(s, vert.texcoord).rg;

    yuv += decode.offset;

    return col * float4(dot(yuv, decode.Rcoeff), dot(yuv, decode.Gcoeff), dot(yuv, decode.Bcoeff), 1.0);
}

fragment float4 SDL_NV21_fragment(CopyVertexOutput vert [[stage_in]],
                                 const device float4 &col [[buffer(0)]],
                                 constant YUVDecode &decode [[buffer(1)]],
                                 texture2d<float> texY [[texture(0)]],
                                 texture2d<float> texUV [[texture(1)]],
                                 sampler s [[sampler(0)]])
{
    float3 yuv;
    yuv.x = texY.sample(s, vert.texcoord).r;
    yuv.yz = texUV.sample(s, vert.texcoord).gr;

    yuv += decode.offset;

    return col * float4(dot(yuv, decode.Rcoeff), dot(yuv, decode.Gcoeff), dot(yuv, decode.Bcoeff), 1.0);
}
