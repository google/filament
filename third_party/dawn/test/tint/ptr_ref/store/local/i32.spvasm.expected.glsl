#version 310 es

void main_1() {
  int i = 0;
  i = 123;
  i = 123;
  i = 123;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_1();
}
