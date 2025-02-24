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
shared mat3 w[4];
void f_inner(uint tint_local_index) {
  {
    uint v_1 = 0u;
    v_1 = tint_local_index;
    while(true) {
      uint v_2 = v_1;
      if ((v_2 >= 4u)) {
        break;
      }
      w[v_2] = mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f));
      {
        v_1 = (v_2 + 1u);
      }
      continue;
    }
  }
  barrier();
  mat3x3_f32_std140 v_3[4] = v.inner;
  mat3 v_4[4] = mat3[4](mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f)), mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f)), mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f)), mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f)));
  {
    uint v_5 = 0u;
    v_5 = 0u;
    while(true) {
      uint v_6 = v_5;
      if ((v_6 >= 4u)) {
        break;
      }
      v_4[v_6] = mat3(v_3[v_6].col0, v_3[v_6].col1, v_3[v_6].col2);
      {
        v_5 = (v_6 + 1u);
      }
      continue;
    }
  }
  w = v_4;
  w[1u] = mat3(v.inner[2u].col0, v.inner[2u].col1, v.inner[2u].col2);
  w[1u][0u] = v.inner[0u].col1.zxy;
  w[1u][0u].x = v.inner[0u].col1.x;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f_inner(gl_LocalInvocationIndex);
}
