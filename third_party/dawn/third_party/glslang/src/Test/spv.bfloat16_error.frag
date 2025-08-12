#version 450 core

#extension GL_EXT_bfloat16 : require
#extension GL_KHR_cooperative_matrix : enable
#extension GL_KHR_memory_scope_semantics : enable
#extension GL_EXT_shader_explicit_arithmetic_types : enable
#extension GL_EXT_scalar_block_layout : enable

layout(location = 0) in bfloat16_t x;
layout(location = 0) out bfloat16_t y;

void main()
{
}

