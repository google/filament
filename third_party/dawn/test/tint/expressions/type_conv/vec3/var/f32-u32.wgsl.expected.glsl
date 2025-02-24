#version 310 es

vec3 u = vec3(1.0f);
uvec3 tint_v3f32_to_v3u32(vec3 value) {
  return mix(uvec3(4294967295u), mix(uvec3(0u), uvec3(value), greaterThanEqual(value, vec3(0.0f))), lessThanEqual(value, vec3(4294967040.0f)));
}
void f() {
  uvec3 v = tint_v3f32_to_v3u32(u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
