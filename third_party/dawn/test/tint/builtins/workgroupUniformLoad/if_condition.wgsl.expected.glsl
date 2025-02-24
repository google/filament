#version 310 es

shared bool v;
int foo() {
  barrier();
  bool v_1 = v;
  barrier();
  if (v_1) {
    return 42;
  }
  return 0;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
