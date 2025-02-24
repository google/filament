#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

float16_t t = 0.0hf;
f16mat4 m() {
  t = (t + 1.0hf);
  return f16mat4(f16vec4(1.0hf, 2.0hf, 3.0hf, 4.0hf), f16vec4(5.0hf, 6.0hf, 7.0hf, 8.0hf), f16vec4(9.0hf, 10.0hf, 11.0hf, 12.0hf), f16vec4(13.0hf, 14.0hf, 15.0hf, 16.0hf));
}
void f() {
  mat4 v = mat4(m());
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
