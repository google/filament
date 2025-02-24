#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

mat4x3 u = mat4x3(vec3(1.0f, 2.0f, 3.0f), vec3(4.0f, 5.0f, 6.0f), vec3(7.0f, 8.0f, 9.0f), vec3(10.0f, 11.0f, 12.0f));
void f() {
  f16mat4x3 v = f16mat4x3(u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
