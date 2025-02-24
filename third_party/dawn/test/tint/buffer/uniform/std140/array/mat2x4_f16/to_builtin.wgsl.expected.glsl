#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct mat2x4_f16_std140 {
  f16vec4 col0;
  f16vec4 col1;
};

layout(binding = 0, std140)
uniform u_block_std140_1_ubo {
  mat2x4_f16_std140 inner[4];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  float16_t inner;
} v_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f16mat4x2 t = transpose(f16mat2x4(v.inner[2u].col0, v.inner[2u].col1));
  float16_t l = length(v.inner[0u].col1.ywxz);
  float16_t a = abs(v.inner[0u].col1.ywxz.x);
  float16_t v_2 = (t[0u].x + float16_t(l));
  v_1.inner = (v_2 + float16_t(a));
}
