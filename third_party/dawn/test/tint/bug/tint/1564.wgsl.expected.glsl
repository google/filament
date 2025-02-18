#version 310 es

void foo() {
  float b = 9.9999461e-41f;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
