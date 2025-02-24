#version 310 es

shared bool v;
bool foo() {
  barrier();
  bool v_1 = v;
  barrier();
  return v_1;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
