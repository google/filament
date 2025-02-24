#version 310 es

shared int v[4];
int[4] foo() {
  barrier();
  int v_1[4] = v;
  barrier();
  return v_1;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
