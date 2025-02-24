#version 310 es

void main_1() {
  float a = 0.0f;
  float b = 0.0f;
  a = 42.0f;
  b = radians(a);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_1();
}
