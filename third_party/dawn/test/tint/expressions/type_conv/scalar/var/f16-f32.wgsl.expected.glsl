#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

float16_t u = 1.0hf;
void f() {
  float v = float(u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
