#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std140)
uniform u_block_std140_1_ubo {
  f16vec2 inner_col0;
  f16vec2 inner_col1;
  f16vec2 inner_col2;
  f16vec2 inner_col3;
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f16mat2x4 t = transpose(f16mat4x2(v.inner_col0, v.inner_col1, v.inner_col2, v.inner_col3));
  float16_t l = length(f16mat4x2(v.inner_col0, v.inner_col1, v.inner_col2, v.inner_col3)[1u]);
  float16_t a = abs(f16mat4x2(v.inner_col0, v.inner_col1, v.inner_col2, v.inner_col3)[0u].yx.x);
}
