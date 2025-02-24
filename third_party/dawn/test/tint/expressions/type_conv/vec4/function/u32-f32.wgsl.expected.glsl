#version 310 es

uint t = 0u;
uvec4 m() {
  t = 1u;
  return uvec4(t);
}
void f() {
  vec4 v = vec4(m());
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
