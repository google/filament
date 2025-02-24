#version 310 es

void f() {
  int a = 1;
  int v = int(a);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
