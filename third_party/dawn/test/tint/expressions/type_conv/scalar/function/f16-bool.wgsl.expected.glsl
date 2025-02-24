#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

float16_t t = 0.0hf;
float16_t m() {
  t = 1.0hf;
  return float16_t(t);
}
void f() {
  bool v = bool(m());
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
