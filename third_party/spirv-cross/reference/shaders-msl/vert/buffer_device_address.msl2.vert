#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct Position;
struct PositionReferences;

struct Position
{
    float2 positions[1];
};

struct Registers
{
    float4x4 view_projection;
    device PositionReferences* references;
};

struct PositionReferences
{
    device Position* buffers[1];
};

struct main0_out
{
    float4 out_color [[user(locn0)]];
    float4 gl_Position [[position]];
};

vertex main0_out main0(constant Registers& registers [[buffer(0)]], uint gl_InstanceIndex [[instance_id]], uint gl_VertexIndex [[vertex_id]])
{
    main0_out out = {};
    int slice = int(gl_InstanceIndex);
    const device Position* __restrict positions = registers.references->buffers[slice];
    float2 pos = positions->positions[int(gl_VertexIndex)] * 2.5;
    pos += ((float2(float(slice % 8), float(slice / 8)) - float2(3.5)) * 3.0);
    out.gl_Position = registers.view_projection * float4(pos, 0.0, 1.0);
    int index_x = int(gl_VertexIndex) % 16;
    int index_y = int(gl_VertexIndex) / 16;
    float r = 0.5 + (0.300000011920928955078125 * sin(float(index_x)));
    float g = 0.5 + (0.300000011920928955078125 * sin(float(index_y)));
    int checkerboard = (index_x ^ index_y) & 1;
    r *= ((float(checkerboard) * 0.800000011920928955078125) + 0.20000000298023223876953125);
    g *= ((float(checkerboard) * 0.800000011920928955078125) + 0.20000000298023223876953125);
    out.out_color = float4(r, g, 0.1500000059604644775390625, 1.0);
    return out;
}

