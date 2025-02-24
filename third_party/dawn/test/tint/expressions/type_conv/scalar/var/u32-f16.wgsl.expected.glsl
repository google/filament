#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

uint u = 1u;
void f() {
  float16_t v = float16_t(u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
