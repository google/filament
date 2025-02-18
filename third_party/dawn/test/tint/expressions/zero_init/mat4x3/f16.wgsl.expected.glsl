#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

void f() {
  f16mat4x3 v = f16mat4x3(f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
