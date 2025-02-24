#version 310 es

float u = 1.0f;
uint tint_f32_to_u32(float value) {
  return mix(4294967295u, mix(0u, uint(value), (value >= 0.0f)), (value <= 4294967040.0f));
}
void f() {
  uint v = tint_f32_to_u32(u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
