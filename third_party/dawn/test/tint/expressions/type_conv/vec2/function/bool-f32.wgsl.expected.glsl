#version 310 es

bool t = false;
bvec2 m() {
  t = true;
  return bvec2(t);
}
void f() {
  vec2 v = vec2(m());
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
