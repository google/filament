#version 310 es

float t = 0.0f;
vec4 m() {
  t = 1.0f;
  return vec4(t);
}
ivec4 tint_v4f32_to_v4i32(vec4 value) {
  return mix(ivec4(2147483647), mix(ivec4((-2147483647 - 1)), ivec4(value), greaterThanEqual(value, vec4(-2147483648.0f))), lessThanEqual(value, vec4(2147483520.0f)));
}
void f() {
  ivec4 v = tint_v4f32_to_v4i32(m());
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
