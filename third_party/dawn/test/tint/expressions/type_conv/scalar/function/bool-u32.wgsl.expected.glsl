#version 310 es

bool t = false;
bool m() {
  t = true;
  return bool(t);
}
void f() {
  uint v = uint(m());
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
