#version 310 es

void deref() {
  mat2x3 a = mat2x3(vec3(0.0f), vec3(0.0f));
  vec3 b = a[0u];
  a[0u] = vec3(1.0f, 2.0f, 3.0f);
}
void no_deref() {
  mat2x3 a = mat2x3(vec3(0.0f), vec3(0.0f));
  vec3 b = a[0u];
  a[0u] = vec3(1.0f, 2.0f, 3.0f);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  deref();
  no_deref();
}
