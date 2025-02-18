#version 310 es

layout(binding = 0, std140)
uniform u_block_std140_1_ubo {
  vec2 inner_col0;
  vec2 inner_col1;
  vec2 inner_col2;
  vec2 inner_col3;
} v;
mat4x2 p = mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f));
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  p = mat4x2(v.inner_col0, v.inner_col1, v.inner_col2, v.inner_col3);
  p[1u] = mat4x2(v.inner_col0, v.inner_col1, v.inner_col2, v.inner_col3)[0u];
  p[1u] = mat4x2(v.inner_col0, v.inner_col1, v.inner_col2, v.inner_col3)[0u].yx;
  p[0u].y = mat4x2(v.inner_col0, v.inner_col1, v.inner_col2, v.inner_col3)[1u].x;
}
