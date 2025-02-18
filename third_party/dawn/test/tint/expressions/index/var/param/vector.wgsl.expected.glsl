#version 310 es

float f(int i) {
  vec3 v = vec3(1.0f, 2.0f, 3.0f);
  return v[min(uint(i), 2u)];
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
