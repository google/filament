#version 310 es

layout(binding = 0, std430)
buffer s_block_1_ssbo {
  int inner;
} v;
int A = 1;
int _A = 2;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int B = A;
  int _B = _A;
  v.inner = (B + _B);
}
