#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

void f() {
  float16_t v[4] = float16_t[4](0.0hf, 0.0hf, 0.0hf, 0.0hf);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
