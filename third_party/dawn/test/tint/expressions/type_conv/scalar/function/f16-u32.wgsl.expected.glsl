#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

float16_t t = 0.0hf;
float16_t m() {
  t = 1.0hf;
  return float16_t(t);
}
uint tint_f16_to_u32(float16_t value) {
  return mix(4294967295u, mix(0u, uint(value), (value >= 0.0hf)), (value <= 65504.0hf));
}
void f() {
  uint v = tint_f16_to_u32(m());
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
