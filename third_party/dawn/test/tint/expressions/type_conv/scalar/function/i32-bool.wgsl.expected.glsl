#version 310 es

int t = 0;
int m() {
  t = 1;
  return int(t);
}
void f() {
  bool v = bool(m());
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
