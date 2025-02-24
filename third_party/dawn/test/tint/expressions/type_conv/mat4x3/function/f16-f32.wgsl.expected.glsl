#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

float16_t t = 0.0hf;
f16mat4x3 m() {
  t = (t + 1.0hf);
  return f16mat4x3(f16vec3(1.0hf, 2.0hf, 3.0hf), f16vec3(4.0hf, 5.0hf, 6.0hf), f16vec3(7.0hf, 8.0hf, 9.0hf), f16vec3(10.0hf, 11.0hf, 12.0hf));
}
void f() {
  mat4x3 v = mat4x3(m());
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
