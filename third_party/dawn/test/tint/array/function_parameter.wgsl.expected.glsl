#version 310 es

layout(binding = 0, std430)
buffer s_block_1_ssbo {
  float inner;
} v;
float f1(float a[4]) {
  return a[3u];
}
float f2(float a[3][4]) {
  return a[2u][3u];
}
float f3(float a[2][3][4]) {
  return a[1u][2u][3u];
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  float a1[4] = float[4](0.0f, 0.0f, 0.0f, 0.0f);
  float a2[3][4] = float[3][4](float[4](0.0f, 0.0f, 0.0f, 0.0f), float[4](0.0f, 0.0f, 0.0f, 0.0f), float[4](0.0f, 0.0f, 0.0f, 0.0f));
  float a3[2][3][4] = float[2][3][4](float[3][4](float[4](0.0f, 0.0f, 0.0f, 0.0f), float[4](0.0f, 0.0f, 0.0f, 0.0f), float[4](0.0f, 0.0f, 0.0f, 0.0f)), float[3][4](float[4](0.0f, 0.0f, 0.0f, 0.0f), float[4](0.0f, 0.0f, 0.0f, 0.0f), float[4](0.0f, 0.0f, 0.0f, 0.0f)));
  float v1 = f1(a1);
  float v2 = f2(a2);
  float v3 = f3(a3);
  v.inner = ((v1 + v2) + v3);
}
