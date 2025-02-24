#version 310 es

layout(binding = 0, std140)
uniform u_block_std140_1_ubo {
  vec2 inner_col0;
  vec2 inner_col1;
  vec2 inner_col2;
} v;
shared mat3x2 w;
void f_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    w = mat3x2(vec2(0.0f), vec2(0.0f), vec2(0.0f));
  }
  barrier();
  w = mat3x2(v.inner_col0, v.inner_col1, v.inner_col2);
  w[1u] = mat3x2(v.inner_col0, v.inner_col1, v.inner_col2)[0u];
  w[1u] = mat3x2(v.inner_col0, v.inner_col1, v.inner_col2)[0u].yx;
  w[0u].y = mat3x2(v.inner_col0, v.inner_col1, v.inner_col2)[1u].x;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f_inner(gl_LocalInvocationIndex);
}
