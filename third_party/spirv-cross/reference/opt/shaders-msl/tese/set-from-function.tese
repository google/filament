#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct Block
{
    float4 a;
    float4 b;
};

struct Foo
{
    float4 a;
    float4 b;
};

struct main0_out
{
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 vColor [[attribute(0)]];
    float4 Block_a [[attribute(2)]];
    float4 Block_b [[attribute(3)]];
};

struct main0_patchIn
{
    float4 vColors [[attribute(1)]];
    float4 Foo_a [[attribute(4)]];
    float4 Foo_b [[attribute(5)]];
    patch_control_point<main0_in> gl_in;
};

[[ patch(quad, 0) ]] vertex main0_out main0(main0_patchIn patchIn [[stage_in]])
{
    main0_out out = {};
    Foo vFoo = {};
    vFoo.a = patchIn.Foo_a;
    vFoo.b = patchIn.Foo_b;
    out.gl_Position = patchIn.gl_in[0].Block_a;
    out.gl_Position += patchIn.gl_in[0].Block_b;
    out.gl_Position += patchIn.gl_in[1].Block_a;
    out.gl_Position += patchIn.gl_in[1].Block_b;
    out.gl_Position += patchIn.gl_in[0].vColor;
    out.gl_Position += patchIn.gl_in[1].vColor;
    out.gl_Position += patchIn.vColors;
    out.gl_Position += vFoo.a;
    out.gl_Position += vFoo.b;
    return out;
}

