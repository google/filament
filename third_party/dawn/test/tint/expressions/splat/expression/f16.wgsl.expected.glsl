#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

void f() {
  f16vec2 v2 = f16vec2(3.0hf);
  f16vec3 v3 = f16vec3(3.0hf);
  f16vec4 v4 = f16vec4(3.0hf);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
