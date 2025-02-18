#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

float t = 0.0f;
mat2 m() {
  t = (t + 1.0f);
  return mat2(vec2(1.0f, 2.0f), vec2(3.0f, 4.0f));
}
void f() {
  f16mat2 v = f16mat2(m());
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
