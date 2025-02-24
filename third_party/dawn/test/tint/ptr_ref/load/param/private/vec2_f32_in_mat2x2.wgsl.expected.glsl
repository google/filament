#version 310 es

mat2 P = mat2(vec2(0.0f), vec2(0.0f));
vec2 func(inout vec2 pointer) {
  return pointer;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  vec2 r = func(P[1u]);
}
