#version 310 es

float t = 0.0f;
vec3 m() {
  t = 1.0f;
  return vec3(t);
}
ivec3 tint_v3f32_to_v3i32(vec3 value) {
  return mix(ivec3(2147483647), mix(ivec3((-2147483647 - 1)), ivec3(value), greaterThanEqual(value, vec3(-2147483648.0f))), lessThanEqual(value, vec3(2147483520.0f)));
}
void f() {
  ivec3 v = tint_v3f32_to_v3i32(m());
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
