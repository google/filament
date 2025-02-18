#version 310 es

void f() {
  int vec3f = 1;
  int b = vec3f;
  vec3 c = vec3(0.0f);
  vec3 d = vec3(0.0f);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
