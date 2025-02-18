#version 310 es

layout(binding = 0, std140)
uniform u_block_std140_1_ubo {
  vec3 inner_col0;
  uint tint_pad_0;
  vec3 inner_col1;
  uint tint_pad_1;
  vec3 inner_col2;
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat3 t = transpose(mat3(v.inner_col0, v.inner_col1, v.inner_col2));
  float l = length(mat3(v.inner_col0, v.inner_col1, v.inner_col2)[1u]);
  float a = abs(mat3(v.inner_col0, v.inner_col1, v.inner_col2)[0u].zxy.x);
}
