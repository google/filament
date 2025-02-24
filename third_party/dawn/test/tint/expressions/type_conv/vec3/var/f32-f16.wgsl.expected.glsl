#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

vec3 u = vec3(1.0f);
void f() {
  f16vec3 v = f16vec3(u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
