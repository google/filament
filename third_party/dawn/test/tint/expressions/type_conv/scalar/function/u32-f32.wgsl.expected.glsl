#version 310 es

uint t = 0u;
uint m() {
  t = 1u;
  return uint(t);
}
void f() {
  float v = float(m());
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
