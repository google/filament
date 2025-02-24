#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

float16_t t = 0.0hf;
f16vec4 m() {
  t = 1.0hf;
  return f16vec4(t);
}
void f() {
  bvec4 v = bvec4(m());
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
