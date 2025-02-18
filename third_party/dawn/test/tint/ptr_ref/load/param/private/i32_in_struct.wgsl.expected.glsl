#version 310 es


struct str {
  int i;
};

str P = str(0);
int func(inout int pointer) {
  return pointer;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int r = func(P.i);
}
