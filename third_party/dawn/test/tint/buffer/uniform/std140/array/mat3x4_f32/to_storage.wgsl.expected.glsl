#version 310 es

layout(binding = 0, std140)
uniform u_block_1_ubo {
  mat3x4 inner[4];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  mat3x4 inner[4];
} v_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v_1.inner = v.inner;
  v_1.inner[1u] = v.inner[2u];
  v_1.inner[1u][0u] = v.inner[0u][1u].ywxz;
  v_1.inner[1u][0u].x = v.inner[0u][1u].x;
}
