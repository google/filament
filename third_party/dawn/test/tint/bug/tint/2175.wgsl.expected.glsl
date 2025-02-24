#version 310 es

layout(binding = 0, std430)
buffer tint_struct_1_ssbo {
  uint member_0;
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.member_0 = 0u;
}
