#version 310 es

void f() {
  int a = 0;
  int b = a;
  int a_1 = 0;
  int b_1 = a_1;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
