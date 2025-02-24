#version 310 es


struct mat3x3_f32_std140 {
  vec3 col0;
  uint tint_pad_0;
  vec3 col1;
  uint tint_pad_1;
  vec3 col2;
  uint tint_pad_2;
};

layout(binding = 0, std140)
uniform u_block_std140_1_ubo {
  mat3x3_f32_std140 inner[4];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  mat3 inner[4];
} v_1;
void tint_store_and_preserve_padding_1(uint target_indices[1], mat3 value_param) {
  v_1.inner[target_indices[0u]][0u] = value_param[0u];
  v_1.inner[target_indices[0u]][1u] = value_param[1u];
  v_1.inner[target_indices[0u]][2u] = value_param[2u];
}
void tint_store_and_preserve_padding(mat3 value_param[4]) {
  {
    uint v_2 = 0u;
    v_2 = 0u;
    while(true) {
      uint v_3 = v_2;
      if ((v_3 >= 4u)) {
        break;
      }
      tint_store_and_preserve_padding_1(uint[1](v_3), value_param[v_3]);
      {
        v_2 = (v_3 + 1u);
      }
      continue;
    }
  }
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat3x3_f32_std140 v_4[4] = v.inner;
  mat3 v_5[4] = mat3[4](mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f)), mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f)), mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f)), mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f)));
  {
    uint v_6 = 0u;
    v_6 = 0u;
    while(true) {
      uint v_7 = v_6;
      if ((v_7 >= 4u)) {
        break;
      }
      v_5[v_7] = mat3(v_4[v_7].col0, v_4[v_7].col1, v_4[v_7].col2);
      {
        v_6 = (v_7 + 1u);
      }
      continue;
    }
  }
  tint_store_and_preserve_padding(v_5);
  mat3 v_8 = mat3(v.inner[2u].col0, v.inner[2u].col1, v.inner[2u].col2);
  tint_store_and_preserve_padding_1(uint[1](1u), v_8);
  v_1.inner[1u][0u] = v.inner[0u].col1.zxy;
  v_1.inner[1u][0u].x = v.inner[0u].col1.x;
}
