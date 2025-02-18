#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uvec4 a = uvec4(1073757184u, 3288351232u, 3296724992u, 987654321u);
  vec4 b = uintBitsToFloat(a);
}
