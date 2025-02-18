#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

bool t = false;
bvec2 m() {
  t = true;
  return bvec2(t);
}
void f() {
  f16vec2 v = f16vec2(m());
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
