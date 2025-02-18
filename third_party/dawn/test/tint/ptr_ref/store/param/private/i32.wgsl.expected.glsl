#version 310 es

int P = 0;
void func(inout int pointer) {
  pointer = 42;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  func(P);
}
