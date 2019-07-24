#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float2 FragColor [[color(0)]];
};

struct main0_in
{
    float3 vUV [[user(locn0)]];
};

void from_function(thread float2& FragColor, thread texture2d<float> uSampler2D, thread const sampler uSampler2DSmplr, thread float3& vUV, thread texture3d<float> uSampler3D, thread const sampler uSampler3DSmplr, thread texturecube<float> uSamplerCube, thread const sampler uSamplerCubeSmplr, thread texture2d<float> uTexture2D, thread sampler uSampler, thread texture3d<float> uTexture3D, thread texturecube<float> uTextureCube)
{
    float2 _22;
    _22.x = uSampler2D.calculate_clamped_lod(uSampler2DSmplr, vUV.xy);
    _22.y = uSampler2D.calculate_unclamped_lod(uSampler2DSmplr, vUV.xy);
    FragColor += _22;
    float2 _31;
    _31.x = uSampler3D.calculate_clamped_lod(uSampler3DSmplr, vUV);
    _31.y = uSampler3D.calculate_unclamped_lod(uSampler3DSmplr, vUV);
    FragColor += _31;
    float2 _40;
    _40.x = uSamplerCube.calculate_clamped_lod(uSamplerCubeSmplr, vUV);
    _40.y = uSamplerCube.calculate_unclamped_lod(uSamplerCubeSmplr, vUV);
    FragColor += _40;
    float2 _53;
    _53.x = uTexture2D.calculate_clamped_lod(uSampler, vUV.xy);
    _53.y = uTexture2D.calculate_unclamped_lod(uSampler, vUV.xy);
    FragColor += _53;
    float2 _62;
    _62.x = uTexture3D.calculate_clamped_lod(uSampler, vUV);
    _62.y = uTexture3D.calculate_unclamped_lod(uSampler, vUV);
    FragColor += _62;
    float2 _71;
    _71.x = uTextureCube.calculate_clamped_lod(uSampler, vUV);
    _71.y = uTextureCube.calculate_unclamped_lod(uSampler, vUV);
    FragColor += _71;
}

fragment main0_out main0(main0_in in [[stage_in]], texture2d<float> uSampler2D [[texture(0)]], texture3d<float> uSampler3D [[texture(1)]], texturecube<float> uSamplerCube [[texture(2)]], texture2d<float> uTexture2D [[texture(3)]], texture3d<float> uTexture3D [[texture(4)]], texturecube<float> uTextureCube [[texture(5)]], sampler uSampler2DSmplr [[sampler(0)]], sampler uSampler3DSmplr [[sampler(1)]], sampler uSamplerCubeSmplr [[sampler(2)]], sampler uSampler [[sampler(3)]])
{
    main0_out out = {};
    out.FragColor = float2(0.0);
    float2 _79;
    _79.x = uSampler2D.calculate_clamped_lod(uSampler2DSmplr, in.vUV.xy);
    _79.y = uSampler2D.calculate_unclamped_lod(uSampler2DSmplr, in.vUV.xy);
    out.FragColor += _79;
    float2 _84;
    _84.x = uSampler3D.calculate_clamped_lod(uSampler3DSmplr, in.vUV);
    _84.y = uSampler3D.calculate_unclamped_lod(uSampler3DSmplr, in.vUV);
    out.FragColor += _84;
    float2 _89;
    _89.x = uSamplerCube.calculate_clamped_lod(uSamplerCubeSmplr, in.vUV);
    _89.y = uSamplerCube.calculate_unclamped_lod(uSamplerCubeSmplr, in.vUV);
    out.FragColor += _89;
    float2 _97;
    _97.x = uTexture2D.calculate_clamped_lod(uSampler, in.vUV.xy);
    _97.y = uTexture2D.calculate_unclamped_lod(uSampler, in.vUV.xy);
    out.FragColor += _97;
    float2 _104;
    _104.x = uTexture3D.calculate_clamped_lod(uSampler, in.vUV);
    _104.y = uTexture3D.calculate_unclamped_lod(uSampler, in.vUV);
    out.FragColor += _104;
    float2 _111;
    _111.x = uTextureCube.calculate_clamped_lod(uSampler, in.vUV);
    _111.y = uTextureCube.calculate_unclamped_lod(uSampler, in.vUV);
    out.FragColor += _111;
    from_function(out.FragColor, uSampler2D, uSampler2DSmplr, in.vUV, uSampler3D, uSampler3DSmplr, uSamplerCube, uSamplerCubeSmplr, uTexture2D, uSampler, uTexture3D, uTextureCube);
    return out;
}

