#version 310 es

layout(binding = 0, std140)
uniform u_block_1_ubo {
  mat4 inner;
} v;
mat4 p = mat4(vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f));
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  p = v.inner;
  p[1u] = v.inner[0u];
  p[1u] = v.inner[0u].ywxz;
  p[0u].y = v.inner[1u].x;
}
