#version 310 es

layout(binding = 0, std140)
uniform u_block_1_ubo {
  mat2x4 inner;
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat4x2 t = transpose(v.inner);
  float l = length(v.inner[1u]);
  float a = abs(v.inner[0u].ywxz.x);
}
