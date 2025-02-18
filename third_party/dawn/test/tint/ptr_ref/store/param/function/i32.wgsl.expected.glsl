#version 310 es

void func(inout int pointer) {
  pointer = 42;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int F = 0;
  func(F);
}
