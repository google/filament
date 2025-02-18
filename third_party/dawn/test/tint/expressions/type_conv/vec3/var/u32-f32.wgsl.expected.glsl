#version 310 es

uvec3 u = uvec3(1u);
void f() {
  vec3 v = vec3(u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
