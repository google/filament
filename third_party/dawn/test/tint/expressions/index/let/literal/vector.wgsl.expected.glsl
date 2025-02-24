#version 310 es

float f() {
  vec3 v = vec3(1.0f, 2.0f, 3.0f);
  return v.y;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
