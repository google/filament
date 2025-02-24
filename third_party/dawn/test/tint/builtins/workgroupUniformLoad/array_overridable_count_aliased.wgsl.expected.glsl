#version 310 es

shared int v[128];
int foo() {
  barrier();
  int v_1[128] = v;
  barrier();
  return v_1[0u];
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
