#version 310 es

shared int v[4];
int foo(uint p_indices[1]) {
  barrier();
  int v_1 = v[p_indices[0u]];
  barrier();
  return v_1;
}
int bar() {
  return foo(uint[1](0u));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
