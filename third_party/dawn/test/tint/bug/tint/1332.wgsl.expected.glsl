#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  float a = 1.24000000953674316406f;
  float b = max(a, 1.17549435e-38f);
}
