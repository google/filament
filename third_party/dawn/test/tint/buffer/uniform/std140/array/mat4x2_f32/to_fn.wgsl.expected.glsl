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
} v_1;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  float inner;
} v_2;
float a(mat4x2 a_1[4]) {
  return a_1[0u][0u].x;
}
float b(mat4x2 m) {
  return m[0u].x;
}
float c(vec2 v) {
  return v.x;
}
float d(float f_1) {
  return f_1;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat4x2_f32_std140 v_3[4] = v_1.inner;
  mat4x2 v_4[4] = mat4x2[4](mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f)), mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f)), mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f)), mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f)));
  {
    uint v_5 = 0u;
    v_5 = 0u;
    while(true) {
      uint v_6 = v_5;
      if ((v_6 >= 4u)) {
        break;
      }
      v_4[v_6] = mat4x2(v_3[v_6].col0, v_3[v_6].col1, v_3[v_6].col2, v_3[v_6].col3);
      {
        v_5 = (v_6 + 1u);
      }
      continue;
    }
  }
  float v_7 = a(v_4);
  float v_8 = (v_7 + b(mat4x2(v_1.inner[1u].col0, v_1.inner[1u].col1, v_1.inner[1u].col2, v_1.inner[1u].col3)));
  float v_9 = (v_8 + c(v_1.inner[1u].col0.yx));
  v_2.inner = (v_9 + d(v_1.inner[1u].col0.yx.x));
}
