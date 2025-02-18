#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  float a = 4.0f;
  vec3 b = vec3(0.0f, 2.0f, 0.0f);
  vec3 r = (a / b);
}
