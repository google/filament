#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

f16vec3 u = f16vec3(1.0hf);
ivec3 tint_v3f16_to_v3i32(f16vec3 value) {
  return mix(ivec3(2147483647), mix(ivec3((-2147483647 - 1)), ivec3(value), greaterThanEqual(value, f16vec3(-65504.0hf))), lessThanEqual(value, f16vec3(65504.0hf)));
}
void f() {
  ivec3 v = tint_v3f16_to_v3i32(u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
