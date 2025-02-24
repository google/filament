#version 310 es

layout(binding = 0, std430)
buffer a_block_1_ssbo {
  uint inner[];
} v;
void v_1() {
  uint v_2 = (uint(v.inner.length()) - 1u);
  uint v_3 = min(uint(1), v_2);
  v.inner[v_3] = (v.inner[v_3] - 1u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
