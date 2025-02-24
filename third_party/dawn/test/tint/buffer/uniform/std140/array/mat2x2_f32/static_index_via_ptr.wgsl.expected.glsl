#version 310 es


struct mat2x2_f32_std140 {
  vec2 col0;
  vec2 col1;
};

layout(binding = 0, std140)
uniform a_block_std140_1_ubo {
  mat2x2_f32_std140 inner[4];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  float inner;
} v_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat2 v_2 = mat2(v.inner[2u].col0, v.inner[2u].col1);
  mat2x2_f32_std140 v_3[4] = v.inner;
  mat2 v_4[4] = mat2[4](mat2(vec2(0.0f), vec2(0.0f)), mat2(vec2(0.0f), vec2(0.0f)), mat2(vec2(0.0f), vec2(0.0f)), mat2(vec2(0.0f), vec2(0.0f)));
  {
    uint v_5 = 0u;
    v_5 = 0u;
    while(true) {
      uint v_6 = v_5;
      if ((v_6 >= 4u)) {
        break;
      }
      v_4[v_6] = mat2(v_3[v_6].col0, v_3[v_6].col1);
      {
        v_5 = (v_6 + 1u);
      }
      continue;
    }
  }
  mat2 l_a[4] = v_4;
  mat2 l_a_i = v_2;
  vec2 l_a_i_i = v_2[1u];
  v_1.inner = (((v_2[1u].x + l_a[0u][0u].x) + l_a_i[0u].x) + l_a_i_i.x);
}
