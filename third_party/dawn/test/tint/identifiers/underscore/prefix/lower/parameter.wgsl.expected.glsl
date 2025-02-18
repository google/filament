#version 310 es

layout(binding = 0, std430)
buffer s_block_1_ssbo {
  int inner;
} v;
void f(int _a) {
  int b = _a;
  v.inner = b;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f(1);
}
