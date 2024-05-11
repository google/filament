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
    const device Position* __restrict positions = registers.references->buffers[int(gl_InstanceIndex)];
    float2 _45 = registers.references->buffers[int(gl_InstanceIndex)]->positions[int(gl_VertexIndex)] * 2.5;
    float2 pos = _45;
    float2 _60 = _45 + ((float2(float(int(gl_InstanceIndex) % 8), float(int(gl_InstanceIndex) / 8)) - float2(3.5)) * 3.0);
    pos = _60;
    out.gl_Position = registers.view_projection * float4(_60, 0.0, 1.0);
    int _82 = int(gl_VertexIndex) % 16;
    int index_x = _82;
    int _85 = int(gl_VertexIndex) / 16;
    int index_y = _85;
    float _92 = sin(float(_82));
    float _94 = fma(0.300000011920928955078125, _92, 0.5);
    float r = _94;
    float _98 = sin(float(_85));
    float _100 = fma(0.300000011920928955078125, _98, 0.5);
    float g = _100;
    int _105 = (_82 ^ _85) & 1;
    int checkerboard = _105;
    float _107 = float(_105);
    float _111 = fma(_107, 0.800000011920928955078125, 0.20000000298023223876953125);
    float _113 = _94 * _111;
    r = _113;
    float _119 = _100 * _111;
    g = _119;
    out.out_color = float4(_113, _119, 0.1500000059604644775390625, 1.0);
    return out;
}

