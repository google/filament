#version 310 es

void f() {
  mat4 v = mat4(vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
