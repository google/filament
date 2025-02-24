#version 310 es


struct S {
  int m;
  uint n[4];
};

uint f() {
  S a[2] = S[2](S(0, uint[4](0u, 0u, 0u, 0u)), S(0, uint[4](0u, 0u, 0u, 0u)));
  return a[1u].n[1u];
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
