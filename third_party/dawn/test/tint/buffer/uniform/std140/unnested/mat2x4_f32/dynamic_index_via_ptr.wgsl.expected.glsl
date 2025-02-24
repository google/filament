#version 310 es

layout(binding = 0, std140)
uniform m_block_1_ubo {
  mat2x4 inner;
} v;
int counter = 0;
int i() {
  counter = (counter + 1);
  return counter;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_1 = min(uint(i()), 1u);
  mat2x4 l_m = v.inner;
  vec4 l_m_i = v.inner[v_1];
}
