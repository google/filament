#version 310 es

uint t = 0u;
uvec3 m() {
  t = 1u;
  return uvec3(t);
}
void f() {
  vec3 v = vec3(m());
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
