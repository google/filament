#version 310 es

float t = 0.0f;
vec2 m() {
  t = 1.0f;
  return vec2(t);
}
void f() {
  bvec2 v = bvec2(m());
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
