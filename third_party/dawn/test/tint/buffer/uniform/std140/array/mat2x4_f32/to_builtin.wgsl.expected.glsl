#version 310 es

layout(binding = 0, std140)
uniform u_block_1_ubo {
  mat2x4 inner[4];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  float inner;
} v_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat4x2 t = transpose(v.inner[2u]);
  float l = length(v.inner[0u][1u].ywxz);
  float a = abs(v.inner[0u][1u].ywxz.x);
  float v_2 = (t[0u].x + float(l));
  v_1.inner = (v_2 + float(a));
}
