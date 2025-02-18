#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  float a = 1.0f;
  float b = 2.0f;
  float r = (a - b);
}
