#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

uint t = 0u;
uvec3 m() {
  t = 1u;
  return uvec3(t);
}
void f() {
  f16vec3 v = f16vec3(m());
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
