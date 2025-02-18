#version 310 es

shared vec4 v;
vec4 foo() {
  barrier();
  vec4 v_1 = v;
  barrier();
  return v_1;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
