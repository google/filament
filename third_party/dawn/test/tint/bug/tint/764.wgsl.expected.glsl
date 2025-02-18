#version 310 es

void f() {
  mat4 m = mat4(vec4(1.0f), vec4(1.0f), vec4(1.0f), vec4(1.0f));
  vec4 v1 = m[0u];
  float a = v1.x;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
