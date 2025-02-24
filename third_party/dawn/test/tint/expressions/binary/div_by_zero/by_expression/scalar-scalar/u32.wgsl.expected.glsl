#version 310 es

uint tint_div_u32(uint lhs, uint rhs) {
  return (lhs / mix(rhs, 1u, (rhs == 0u)));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint a = 1u;
  uint b = 0u;
  uint r = tint_div_u32(a, (b + b));
}
