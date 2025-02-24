#version 310 es

vec2 u = vec2(1.0f);
uvec2 tint_v2f32_to_v2u32(vec2 value) {
  return mix(uvec2(4294967295u), mix(uvec2(0u), uvec2(value), greaterThanEqual(value, vec2(0.0f))), lessThanEqual(value, vec2(4294967040.0f)));
}
void f() {
  uvec2 v = tint_v2f32_to_v2u32(u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
