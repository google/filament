#version 310 es

void c(int z) {
  int a = (1 + z);
  a = (a + 2);
}
void b() {
  c(2);
  c(3);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
