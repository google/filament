#version 310 es

layout(binding = 0, std140)
uniform u_block_std140_1_ubo {
  vec3 inner_col0;
  uint tint_pad_0;
  vec3 inner_col1;
} v;
shared mat2x3 w;
void f_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    w = mat2x3(vec3(0.0f), vec3(0.0f));
  }
  barrier();
  w = mat2x3(v.inner_col0, v.inner_col1);
  w[1u] = mat2x3(v.inner_col0, v.inner_col1)[0u];
  w[1u] = mat2x3(v.inner_col0, v.inner_col1)[0u].zxy;
  w[0u].y = mat2x3(v.inner_col0, v.inner_col1)[1u].x;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f_inner(gl_LocalInvocationIndex);
}
