#version 310 es

layout(binding = 1, std430)
buffer data_block_1_ssbo {
  int inner[];
} v;
int foo() {
  uint v_1 = (uint(v.inner.length()) - 1u);
  uint v_2 = min(uint(0), v_1);
  return v.inner[v_2];
}
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
void main() {
  foo();
}
