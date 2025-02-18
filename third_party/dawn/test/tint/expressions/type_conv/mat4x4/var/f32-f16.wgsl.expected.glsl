#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

mat4 u = mat4(vec4(1.0f, 2.0f, 3.0f, 4.0f), vec4(5.0f, 6.0f, 7.0f, 8.0f), vec4(9.0f, 10.0f, 11.0f, 12.0f), vec4(13.0f, 14.0f, 15.0f, 16.0f));
void f() {
  f16mat4 v = f16mat4(u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
