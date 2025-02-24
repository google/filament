#version 310 es

layout(binding = 0, std140)
uniform u_block_1_ubo {
  mat2x4 inner[4];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  float inner;
} v_1;
mat2x4 p[4] = mat2x4[4](mat2x4(vec4(0.0f), vec4(0.0f)), mat2x4(vec4(0.0f), vec4(0.0f)), mat2x4(vec4(0.0f), vec4(0.0f)), mat2x4(vec4(0.0f), vec4(0.0f)));
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  p = v.inner;
  p[1u] = v.inner[2u];
  p[1u][0u] = v.inner[0u][1u].ywxz;
  p[1u][0u].x = v.inner[0u][1u].x;
  v_1.inner = p[1u][0u].x;
}
