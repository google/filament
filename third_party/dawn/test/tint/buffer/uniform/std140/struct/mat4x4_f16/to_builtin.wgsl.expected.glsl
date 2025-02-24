#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct S_std140 {
  int before;
  uint tint_pad_0;
  f16vec4 m_col0;
  f16vec4 m_col1;
  f16vec4 m_col2;
  f16vec4 m_col3;
  uint tint_pad_1;
  uint tint_pad_2;
  uint tint_pad_3;
  uint tint_pad_4;
  uint tint_pad_5;
  uint tint_pad_6;
  int after;
  uint tint_pad_7;
  uint tint_pad_8;
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
};

layout(binding = 0, std140)
uniform u_block_std140_1_ubo {
  S_std140 inner[4];
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f16mat4 t = transpose(f16mat4(v.inner[2u].m_col0, v.inner[2u].m_col1, v.inner[2u].m_col2, v.inner[2u].m_col3));
  float16_t l = length(v.inner[0u].m_col1.ywxz);
  float16_t a = abs(v.inner[0u].m_col1.ywxz.x);
}
