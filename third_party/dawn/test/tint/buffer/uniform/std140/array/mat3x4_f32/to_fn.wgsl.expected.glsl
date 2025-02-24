#version 310 es

layout(binding = 0, std140)
uniform u_block_1_ubo {
  mat3x4 inner[4];
} v_1;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  float inner;
} v_2;
float a(mat3x4 a_1[4]) {
  return a_1[0u][0u].x;
}
float b(mat3x4 m) {
  return m[0u].x;
}
float c(vec4 v) {
  return v.x;
}
float d(float f_1) {
  return f_1;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  float v_3 = a(v_1.inner);
  float v_4 = (v_3 + b(v_1.inner[1u]));
  float v_5 = (v_4 + c(v_1.inner[1u][0u].ywxz));
  v_2.inner = (v_5 + d(v_1.inner[1u][0u].ywxz.x));
}
