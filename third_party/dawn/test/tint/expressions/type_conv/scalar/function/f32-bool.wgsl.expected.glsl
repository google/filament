#version 310 es

float t = 0.0f;
float m() {
  t = 1.0f;
  return float(t);
}
void f() {
  bool v = bool(m());
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
