#version 310 es

int c(int z) {
  int a = (1 + z);
  a = (a + 2);
  return a;
}
void b() {
  int b_1 = c(2);
  int v = c(3);
  b_1 = (b_1 + v);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
