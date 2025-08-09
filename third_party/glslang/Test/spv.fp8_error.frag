#version 450 core

#extension GL_EXT_float_e5m2 : require
#extension GL_EXT_float_e4m3 : require
#extension GL_KHR_cooperative_matrix : enable
#extension GL_KHR_memory_scope_semantics : enable
#extension GL_EXT_shader_explicit_arithmetic_types : enable
#extension GL_EXT_scalar_block_layout : enable

layout(location = 0) in floate5m2_t x0;
layout(location = 0) out floate5m2_t y0;
layout(location = 1) in floate4m3_t x1;
layout(location = 1) out floate4m3_t y1;

void main()
{
    floate5m2_t x;
    floate4m3_t y;
    x = y;
    y = x;
}
