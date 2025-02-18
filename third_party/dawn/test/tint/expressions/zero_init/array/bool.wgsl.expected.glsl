#version 310 es

void f() {
  bool v[4] = bool[4](false, false, false, false);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
