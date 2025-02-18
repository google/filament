#version 310 es

float b(int i) {
  return 2.29999995231628417969f;
}
int c(uint u) {
  return 1;
}
void a() {
  float a_1 = b(c(2u));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
