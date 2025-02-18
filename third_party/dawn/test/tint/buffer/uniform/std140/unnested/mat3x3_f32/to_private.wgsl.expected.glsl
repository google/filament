#version 310 es

layout(binding = 0, std140)
uniform u_block_std140_1_ubo {
  vec3 inner_col0;
  uint tint_pad_0;
  vec3 inner_col1;
  uint tint_pad_1;
  vec3 inner_col2;
} v;
mat3 p = mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f));
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  p = mat3(v.inner_col0, v.inner_col1, v.inner_col2);
  p[1u] = mat3(v.inner_col0, v.inner_col1, v.inner_col2)[0u];
  p[1u] = mat3(v.inner_col0, v.inner_col1, v.inner_col2)[0u].zxy;
  p[0u].y = mat3(v.inner_col0, v.inner_col1, v.inner_col2)[1u].x;
}
