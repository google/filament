#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

fragment main0_out main0(texture_buffer<float> buf [[texture(0)]], texture_buffer<float, access::write> bufOut [[texture(1)]], float4 gl_FragCoord [[position]])
{
    main0_out out = {};
    out.FragColor = buf.read(uint(0));
    bufOut.write(out.FragColor, uint(int(gl_FragCoord.x)));
    return out;
}

