#version 310 es


struct mat2x2_f32_std140 {
  vec2 col0;
  vec2 col1;
};

layout(binding = 0, std140)
uniform u_block_std140_1_ubo {
  mat2x2_f32_std140 inner[4];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  float inner;
} v_1;
mat2 p[4] = mat2[4](mat2(vec2(0.0f), vec2(0.0f)), mat2(vec2(0.0f), vec2(0.0f)), mat2(vec2(0.0f), vec2(0.0f)), mat2(vec2(0.0f), vec2(0.0f)));
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat2x2_f32_std140 v_2[4] = v.inner;
  mat2 v_3[4] = mat2[4](mat2(vec2(0.0f), vec2(0.0f)), mat2(vec2(0.0f), vec2(0.0f)), mat2(vec2(0.0f), vec2(0.0f)), mat2(vec2(0.0f), vec2(0.0f)));
  {
    uint v_4 = 0u;
    v_4 = 0u;
    while(true) {
      uint v_5 = v_4;
      if ((v_5 >= 4u)) {
        break;
      }
      v_3[v_5] = mat2(v_2[v_5].col0, v_2[v_5].col1);
      {
        v_4 = (v_5 + 1u);
      }
      continue;
    }
  }
  p = v_3;
  p[1u] = mat2(v.inner[2u].col0, v.inner[2u].col1);
  p[1u][0u] = v.inner[0u].col1.yx;
  p[1u][0u].x = v.inner[0u].col1.x;
  v_1.inner = p[1u][0u].x;
}
