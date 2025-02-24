#version 310 es


struct mat2x3_f32_std140 {
  vec3 col0;
  uint tint_pad_0;
  vec3 col1;
  uint tint_pad_1;
};

layout(binding = 0, std140)
uniform u_block_std140_1_ubo {
  mat2x3_f32_std140 inner[4];
} v_1;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  float inner;
} v_2;
float a(mat2x3 a_1[4]) {
  return a_1[0u][0u].x;
}
float b(mat2x3 m) {
  return m[0u].x;
}
float c(vec3 v) {
  return v.x;
}
float d(float f_1) {
  return f_1;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat2x3_f32_std140 v_3[4] = v_1.inner;
  mat2x3 v_4[4] = mat2x3[4](mat2x3(vec3(0.0f), vec3(0.0f)), mat2x3(vec3(0.0f), vec3(0.0f)), mat2x3(vec3(0.0f), vec3(0.0f)), mat2x3(vec3(0.0f), vec3(0.0f)));
  {
    uint v_5 = 0u;
    v_5 = 0u;
    while(true) {
      uint v_6 = v_5;
      if ((v_6 >= 4u)) {
        break;
      }
      v_4[v_6] = mat2x3(v_3[v_6].col0, v_3[v_6].col1);
      {
        v_5 = (v_6 + 1u);
      }
      continue;
    }
  }
  float v_7 = a(v_4);
  float v_8 = (v_7 + b(mat2x3(v_1.inner[1u].col0, v_1.inner[1u].col1)));
  float v_9 = (v_8 + c(v_1.inner[1u].col0.zxy));
  v_2.inner = (v_9 + d(v_1.inner[1u].col0.zxy.x));
}
