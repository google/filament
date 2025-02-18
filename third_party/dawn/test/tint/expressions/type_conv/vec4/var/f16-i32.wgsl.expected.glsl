#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

f16vec4 u = f16vec4(1.0hf);
ivec4 tint_v4f16_to_v4i32(f16vec4 value) {
  return mix(ivec4(2147483647), mix(ivec4((-2147483647 - 1)), ivec4(value), greaterThanEqual(value, f16vec4(-65504.0hf))), lessThanEqual(value, f16vec4(65504.0hf)));
}
void f() {
  ivec4 v = tint_v4f16_to_v4i32(u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
