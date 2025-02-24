#version 310 es

layout(binding = 0, std430)
buffer S_block_1_ssbo {
  int inner;
} v;
void func() {
  v.inner = 42;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  func();
}
