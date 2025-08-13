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
    out.gl_Position = registers.view_projection * float4((registers.references->buffers[int(gl_InstanceIndex)]->positions[int(gl_VertexIndex)] * 2.5) + ((float2(float(int(gl_InstanceIndex) % 8), float(int(gl_InstanceIndex) / 8)) - float2(3.5)) * 3.0), 0.0, 1.0);
    int _82 = int(gl_VertexIndex) % 16;
    int _85 = int(gl_VertexIndex) / 16;
    float _111 = fma(float((_82 ^ _85) & 1), 0.800000011920928955078125, 0.20000000298023223876953125);
    out.out_color = float4(fma(0.300000011920928955078125, sin(float(_82)), 0.5) * _111, fma(0.300000011920928955078125, sin(float(_85)), 0.5) * _111, 0.1500000059604644775390625, 1.0);
    return out;
}

