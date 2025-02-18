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
  float inner;
} v_1;
mat3 p[4] = mat3[4](mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f)), mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f)), mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f)), mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f)));
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat3x3_f32_std140 v_2[4] = v.inner;
  mat3 v_3[4] = mat3[4](mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f)), mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f)), mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f)), mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f)));
  {
    uint v_4 = 0u;
    v_4 = 0u;
    while(true) {
      uint v_5 = v_4;
      if ((v_5 >= 4u)) {
        break;
      }
      v_3[v_5] = mat3(v_2[v_5].col0, v_2[v_5].col1, v_2[v_5].col2);
      {
        v_4 = (v_5 + 1u);
      }
      continue;
    }
  }
  p = v_3;
  p[1u] = mat3(v.inner[2u].col0, v.inner[2u].col1, v.inner[2u].col2);
  p[1u][0u] = v.inner[0u].col1.zxy;
  p[1u][0u].x = v.inner[0u].col1.x;
  v_1.inner = p[1u][0u].x;
}
