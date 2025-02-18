#version 310 es

layout(binding = 0, std140)
uniform u_block_std140_1_ubo {
  vec2 inner_col0;
  vec2 inner_col1;
  vec2 inner_col2;
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat2x3 t = transpose(mat3x2(v.inner_col0, v.inner_col1, v.inner_col2));
  float l = length(mat3x2(v.inner_col0, v.inner_col1, v.inner_col2)[1u]);
  float a = abs(mat3x2(v.inner_col0, v.inner_col1, v.inner_col2)[0u].yx.x);
}
