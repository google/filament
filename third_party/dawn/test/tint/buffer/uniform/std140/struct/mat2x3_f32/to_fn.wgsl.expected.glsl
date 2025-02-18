#version 310 es


struct S_std140 {
  int before;
  uint tint_pad_0;
  uint tint_pad_1;
  uint tint_pad_2;
  vec3 m_col0;
  uint tint_pad_3;
  vec3 m_col1;
  uint tint_pad_4;
  uint tint_pad_5;
  uint tint_pad_6;
  uint tint_pad_7;
  uint tint_pad_8;
  int after;
  uint tint_pad_9;
  uint tint_pad_10;
  uint tint_pad_11;
  uint tint_pad_12;
  uint tint_pad_13;
  uint tint_pad_14;
  uint tint_pad_15;
  uint tint_pad_16;
  uint tint_pad_17;
  uint tint_pad_18;
  uint tint_pad_19;
  uint tint_pad_20;
  uint tint_pad_21;
  uint tint_pad_22;
  uint tint_pad_23;
};

struct S {
  int before;
  mat2x3 m;
  int after;
};

layout(binding = 0, std140)
uniform u_block_std140_1_ubo {
  S_std140 inner[4];
} v_1;
void a(S a_1[4]) {
}
void b(S s) {
}
void c(mat2x3 m) {
}
void d(vec3 v) {
}
void e(float f_1) {
}
S tint_convert_S(S_std140 tint_input) {
  return S(tint_input.before, mat2x3(tint_input.m_col0, tint_input.m_col1), tint_input.after);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  S_std140 v_2[4] = v_1.inner;
  S v_3[4] = S[4](S(0, mat2x3(vec3(0.0f), vec3(0.0f)), 0), S(0, mat2x3(vec3(0.0f), vec3(0.0f)), 0), S(0, mat2x3(vec3(0.0f), vec3(0.0f)), 0), S(0, mat2x3(vec3(0.0f), vec3(0.0f)), 0));
  {
    uint v_4 = 0u;
    v_4 = 0u;
    while(true) {
      uint v_5 = v_4;
      if ((v_5 >= 4u)) {
        break;
      }
      v_3[v_5] = tint_convert_S(v_2[v_5]);
      {
        v_4 = (v_5 + 1u);
      }
      continue;
    }
  }
  a(v_3);
  b(tint_convert_S(v_1.inner[2u]));
  c(mat2x3(v_1.inner[2u].m_col0, v_1.inner[2u].m_col1));
  d(v_1.inner[0u].m_col1.zxy);
  e(v_1.inner[0u].m_col1.zxy.x);
}
