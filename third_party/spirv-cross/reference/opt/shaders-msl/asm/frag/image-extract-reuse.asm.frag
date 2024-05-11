#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    int2 Size [[color(0)]];
};

fragment main0_out main0(texture2d<float> uTexture [[texture(0)]], sampler uTextureSmplr [[sampler(0)]])
{
    main0_out out = {};
    out.Size = int2(uTexture.get_width(), uTexture.get_height()) + int2(uTexture.get_width(1), uTexture.get_height(1));
    return out;
}

