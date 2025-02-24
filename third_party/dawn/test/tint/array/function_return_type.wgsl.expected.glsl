#version 310 es

layout(binding = 0, std430)
buffer s_block_1_ssbo {
  float inner;
} v;
float[4] f1() {
  return float[4](0.0f, 0.0f, 0.0f, 0.0f);
}
float[3][4] f2() {
  float v_1[4] = f1();
  float v_2[4] = f1();
  return float[3][4](v_1, v_2, f1());
}
float[2][3][4] f3() {
  float v_3[3][4] = f2();
  return float[2][3][4](v_3, f2());
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  float a1[4] = f1();
  float a2[3][4] = f2();
  float a3[2][3][4] = f3();
  v.inner = ((a1[0u] + a2[0u][0u]) + a3[0u][0u][0u]);
}
