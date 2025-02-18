#version 310 es

void main_1() {
  mat3 m = mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f));
  m = mat3(vec3(1.0f, 2.0f, 3.0f), vec3(4.0f, 5.0f, 6.0f), vec3(7.0f, 8.0f, 9.0f));
  m[1u] = vec3(5.0f);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_1();
}
