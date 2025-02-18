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
int counter = 0;
int i() {
  counter = (counter + 1);
  return counter;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_2 = min(uint(i()), 3u);
  mat2 v_3 = mat2(v.inner[v_2].col0, v.inner[v_2].col1);
  vec2 v_4 = v_3[min(uint(i()), 1u)];
  mat2x2_f32_std140 v_5[4] = v.inner;
  mat2 v_6[4] = mat2[4](mat2(vec2(0.0f), vec2(0.0f)), mat2(vec2(0.0f), vec2(0.0f)), mat2(vec2(0.0f), vec2(0.0f)), mat2(vec2(0.0f), vec2(0.0f)));
  {
    uint v_7 = 0u;
    v_7 = 0u;
    while(true) {
      uint v_8 = v_7;
      if ((v_8 >= 4u)) {
        break;
      }
      v_6[v_8] = mat2(v_5[v_8].col0, v_5[v_8].col1);
      {
        v_7 = (v_8 + 1u);
      }
      continue;
    }
  }
  mat2 l_a[4] = v_6;
  mat2 l_a_i = v_3;
  vec2 l_a_i_i = v_4;
  v_1.inner = (((v_4.x + l_a[0u][0u].x) + l_a_i[0u].x) + l_a_i_i.x);
}
