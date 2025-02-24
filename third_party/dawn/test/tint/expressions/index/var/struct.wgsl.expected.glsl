#version 310 es


struct S {
  int m;
  uint n;
};

uint f() {
  S a = S(0, 0u);
  return a.n;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
