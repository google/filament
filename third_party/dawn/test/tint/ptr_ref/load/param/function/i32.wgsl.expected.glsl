#version 310 es

int func(inout int pointer) {
  return pointer;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int F = 0;
  int r = func(F);
}
