#version 310 es

layout(binding = 0, std140)
uniform S_block_1_ubo {
  int inner;
} v;
int func() {
  return v.inner;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int r = func();
}
