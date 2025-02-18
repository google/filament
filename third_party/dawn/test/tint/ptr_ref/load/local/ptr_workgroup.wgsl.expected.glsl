#version 310 es

shared int i;
void main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    i = 0;
  }
  barrier();
  i = 123;
  int u = (i + 1);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_LocalInvocationIndex);
}
