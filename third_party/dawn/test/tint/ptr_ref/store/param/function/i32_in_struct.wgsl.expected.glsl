#version 310 es


struct str {
  int i;
};

void func(inout int pointer) {
  pointer = 42;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  str F = str(0);
  func(F.i);
}
