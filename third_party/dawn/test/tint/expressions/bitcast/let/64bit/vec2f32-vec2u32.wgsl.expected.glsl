#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  vec2 a = vec2(2.003662109375f, -513.03125f);
  uvec2 b = floatBitsToUint(a);
}
