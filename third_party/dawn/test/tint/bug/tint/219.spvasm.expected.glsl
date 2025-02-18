#version 310 es

float x_200(inout vec2 x_201) {
  float x_212 = x_201.x;
  return x_212;
}
void main_1() {
  vec2 x_11 = vec2(0.0f);
  float x_12 = x_200(x_11);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_1();
}
