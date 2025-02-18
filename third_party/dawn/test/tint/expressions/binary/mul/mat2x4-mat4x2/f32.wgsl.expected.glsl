#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat2x4 a = mat2x4(vec4(1.0f, 2.0f, 3.0f, 4.0f), vec4(5.0f, 6.0f, 7.0f, 8.0f));
  mat4x2 b = mat4x2(vec2(-1.0f, -2.0f), vec2(-3.0f, -4.0f), vec2(-5.0f, -6.0f), vec2(-7.0f, -8.0f));
  mat4 r = (a * b);
}
