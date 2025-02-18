#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint a = 1073757184u;
  float b = uintBitsToFloat(a);
}
