#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std140)
uniform u_block_std140_1_ubo {
  f16vec2 inner_col0;
  f16vec2 inner_col1;
  f16vec2 inner_col2;
  f16vec2 inner_col3;
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  f16mat4x2 inner;
} v_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f16mat4x2 x = f16mat4x2(v.inner_col0, v.inner_col1, v.inner_col2, v.inner_col3);
  v_1.inner = x;
}
