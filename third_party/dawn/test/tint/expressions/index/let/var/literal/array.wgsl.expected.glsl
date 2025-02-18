#version 310 es

int f() {
  int a[8] = int[8](1, 2, 3, 4, 5, 6, 7, 8);
  int i = 1;
  return a[min(uint(i), 7u)];
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
