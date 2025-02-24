#version 310 es

int f(int x) {
  int a[8] = int[8](1, 2, 3, 4, 5, 6, 7, 8);
  int i = x;
  uint v = min(uint(i), 7u);
  return a[v];
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
