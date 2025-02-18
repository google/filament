#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

float16_t get_f16() {
  return 1.0hf;
}
void f() {
  f16vec2 v2 = f16vec2(get_f16());
  f16vec3 v3 = f16vec3(get_f16());
  f16vec4 v4 = f16vec4(get_f16());
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
