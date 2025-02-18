#version 310 es

int u = 1;
void f() {
  bool v = bool(u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
