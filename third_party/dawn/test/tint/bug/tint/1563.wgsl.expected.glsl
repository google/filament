#version 310 es

float foo() {
  int oob = 99;
  float b = vec4(0.0f)[min(uint(oob), 3u)];
  vec4 v = vec4(0.0f);
  v[min(uint(oob), 3u)] = b;
  return b;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
