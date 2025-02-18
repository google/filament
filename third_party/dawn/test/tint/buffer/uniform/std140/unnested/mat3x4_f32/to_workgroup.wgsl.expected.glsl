#version 310 es

layout(binding = 0, std140)
uniform u_block_1_ubo {
  mat3x4 inner;
} v;
shared mat3x4 w;
void f_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    w = mat3x4(vec4(0.0f), vec4(0.0f), vec4(0.0f));
  }
  barrier();
  w = v.inner;
  w[1u] = v.inner[0u];
  w[1u] = v.inner[0u].ywxz;
  w[0u].y = v.inner[1u].x;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f_inner(gl_LocalInvocationIndex);
}
