#version 310 es


struct str {
  int i;
};

layout(binding = 0, std430)
buffer S_block_1_ssbo {
  str inner[4];
} v;
str func(uint pointer_indices[1]) {
  return v.inner[pointer_indices[0u]];
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  str r = func(uint[1](2u));
}
