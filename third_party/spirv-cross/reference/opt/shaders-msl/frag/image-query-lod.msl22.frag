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
    float2 _119;
    _119.x = uSampler2D.calculate_clamped_lod(uSampler2DSmplr, in.vUV.xy);
    _119.y = uSampler2D.calculate_unclamped_lod(uSampler2DSmplr, in.vUV.xy);
    out.FragColor += _119;
    float2 _124;
    _124.x = uSampler3D.calculate_clamped_lod(uSampler3DSmplr, in.vUV);
    _124.y = uSampler3D.calculate_unclamped_lod(uSampler3DSmplr, in.vUV);
    out.FragColor += _124;
    float2 _129;
    _129.x = uSamplerCube.calculate_clamped_lod(uSamplerCubeSmplr, in.vUV);
    _129.y = uSamplerCube.calculate_unclamped_lod(uSamplerCubeSmplr, in.vUV);
    out.FragColor += _129;
    float2 _137;
    _137.x = uTexture2D.calculate_clamped_lod(uSampler, in.vUV.xy);
    _137.y = uTexture2D.calculate_unclamped_lod(uSampler, in.vUV.xy);
    out.FragColor += _137;
    float2 _144;
    _144.x = uTexture3D.calculate_clamped_lod(uSampler, in.vUV);
    _144.y = uTexture3D.calculate_unclamped_lod(uSampler, in.vUV);
    out.FragColor += _144;
    float2 _151;
    _151.x = uTextureCube.calculate_clamped_lod(uSampler, in.vUV);
    _151.y = uTextureCube.calculate_unclamped_lod(uSampler, in.vUV);
    out.FragColor += _151;
    return out;
}

