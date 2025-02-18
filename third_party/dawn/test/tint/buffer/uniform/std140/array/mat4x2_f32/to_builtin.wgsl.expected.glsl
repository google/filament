#version 310 es


struct mat4x2_f32_std140 {
  vec2 col0;
  vec2 col1;
  vec2 col2;
  vec2 col3;
};

layout(binding = 0, std140)
uniform u_block_std140_1_ubo {
  mat4x2_f32_std140 inner[4];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  float inner;
} v_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat2x4 t = transpose(mat4x2(v.inner[2u].col0, v.inner[2u].col1, v.inner[2u].col2, v.inner[2u].col3));
  float l = length(v.inner[0u].col1.yx);
  float a = abs(v.inner[0u].col1.yx.x);
  float v_2 = (t[0u].x + float(l));
  v_1.inner = (v_2 + float(a));
}
