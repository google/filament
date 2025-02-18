#version 310 es

void f() {
  mat2x3 v = mat2x3(vec3(0.0f), vec3(0.0f));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
