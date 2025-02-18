#version 310 es


struct S {
  float f;
};

layout(binding = 0, std430)
buffer arr_block_1_ssbo {
  S inner[];
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint len = uint(v.inner.length());
}
