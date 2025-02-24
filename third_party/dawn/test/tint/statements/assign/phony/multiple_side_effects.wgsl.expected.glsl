#version 310 es

int f(int a, int b, int c) {
  return ((a * b) + c);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int v = f(1, 2, 3);
  int v_1 = f(4, 5, 6);
  int v_2 = (v + (v_1 * f(7, f(8, 9, 10), 11)));
}
