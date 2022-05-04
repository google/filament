#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct Block
{
    float4 a;
    float4 b;
};

struct PatchBlock
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
    float4 blocks_a [[attribute(4)]];
    float4 blocks_b [[attribute(5)]];
    float4 vFoos_a [[attribute(14)]];
    float4 vFoos_b [[attribute(15)]];
};

struct main0_patchIn
{
    float4 vColors [[attribute(1)]];
    float4 patch_block_a [[attribute(6)]];
    float4 patch_block_b [[attribute(7)]];
    float4 vFoo_a [[attribute(8)]];
    float4 vFoo_b [[attribute(9)]];
    patch_control_point<main0_in> gl_in;
};

static inline __attribute__((always_inline))
void set_from_function(thread float4& gl_Position, thread patch_control_point<main0_in>& gl_in, thread PatchBlock& patch_block, thread float4& vColors, thread Foo& vFoo)
{
    gl_Position = gl_in[0].blocks_a;
    gl_Position += gl_in[0].blocks_b;
    gl_Position += gl_in[1].blocks_a;
    gl_Position += gl_in[1].blocks_b;
    gl_Position += patch_block.a;
    gl_Position += patch_block.b;
    gl_Position += gl_in[0].vColor;
    gl_Position += gl_in[1].vColor;
    gl_Position += vColors;
    Foo foo = vFoo;
    gl_Position += foo.a;
    gl_Position += foo.b;
    Foo _106 = Foo{ gl_in[0].vFoos_a, gl_in[0].vFoos_b };
    foo = _106;
    gl_Position += foo.a;
    gl_Position += foo.b;
    Foo _120 = Foo{ gl_in[1].vFoos_a, gl_in[1].vFoos_b };
    foo = _120;
    gl_Position += foo.a;
    gl_Position += foo.b;
}

[[ patch(quad, 0) ]] vertex main0_out main0(main0_patchIn patchIn [[stage_in]])
{
    main0_out out = {};
    PatchBlock patch_block = {};
    Foo vFoo = {};
    patch_block.a = patchIn.patch_block_a;
    patch_block.b = patchIn.patch_block_b;
    vFoo.a = patchIn.vFoo_a;
    vFoo.b = patchIn.vFoo_b;
    set_from_function(out.gl_Position, patchIn.gl_in, patch_block, patchIn.vColors, vFoo);
    return out;
}

