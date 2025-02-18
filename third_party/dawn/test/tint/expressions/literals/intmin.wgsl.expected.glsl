#version 310 es

int add_int_min_explicit() {
  int a = (-2147483647 - 1);
  int b = (a + 1);
  int c = -2147483647;
  return c;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
