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
    float4 vColor;
    float4 blocks_a;
    float4 blocks_b;
    Foo vFoos;
};

struct main0_patchIn
{
    float4 vColors;
    float4 patch_block_a;
    float4 patch_block_b;
    Foo vFoo;
};

static inline __attribute__((always_inline))
void set_from_function(thread float4& gl_Position, const device main0_in* thread & gl_in, thread PatchBlock& patch_block, const device float4& vColors, const device Foo& vFoo)
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
    foo = gl_in[0].vFoos;
    gl_Position += foo.a;
    gl_Position += foo.b;
    foo = gl_in[1].vFoos;
    gl_Position += foo.a;
    gl_Position += foo.b;
}

[[ patch(quad, 0) ]] vertex main0_out main0(uint gl_PrimitiveID [[patch_id]], const device main0_patchIn* spvPatchIn [[buffer(20)]], const device main0_in* spvIn [[buffer(22)]])
{
    main0_out out = {};
    PatchBlock patch_block = {};
    const device main0_in* gl_in = &spvIn[gl_PrimitiveID * 0];
    const device main0_patchIn& patchIn = spvPatchIn[gl_PrimitiveID];
    patch_block.a = patchIn.patch_block_a;
    patch_block.b = patchIn.patch_block_b;
    set_from_function(out.gl_Position, gl_in, patch_block, patchIn.vColors, patchIn.vFoo);
    return out;
}

