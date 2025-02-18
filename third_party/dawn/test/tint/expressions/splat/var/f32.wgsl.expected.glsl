#version 310 es

void f() {
  float v = 3.0f;
  vec2 v2 = vec2(v);
  vec3 v3 = vec3(v);
  vec4 v4 = vec4(v);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
