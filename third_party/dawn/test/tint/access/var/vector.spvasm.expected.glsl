#version 310 es

void main_1() {
  vec3 v = vec3(0.0f);
  float x_14 = v.y;
  vec2 x_17 = v.xz;
  vec3 x_19 = v.xzy;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_1();
}
