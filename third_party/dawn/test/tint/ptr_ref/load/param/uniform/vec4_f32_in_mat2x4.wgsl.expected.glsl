#version 310 es

layout(binding = 0, std140)
uniform S_block_1_ubo {
  mat2x4 inner;
} v;
vec4 func(uint pointer_indices[1]) {
  return v.inner[pointer_indices[0u]];
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  vec4 r = func(uint[1](1u));
}
