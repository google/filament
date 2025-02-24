#version 310 es

int t = 0;
ivec2 m() {
  t = 1;
  return ivec2(t);
}
void f() {
  uvec2 v = uvec2(m());
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
