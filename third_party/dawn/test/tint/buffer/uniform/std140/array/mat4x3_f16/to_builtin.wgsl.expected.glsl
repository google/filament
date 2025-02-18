#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct mat4x3_f16_std140 {
  f16vec3 col0;
  f16vec3 col1;
  f16vec3 col2;
  f16vec3 col3;
};

layout(binding = 0, std140)
uniform u_block_std140_1_ubo {
  mat4x3_f16_std140 inner[4];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  float16_t inner;
} v_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f16mat3x4 t = transpose(f16mat4x3(v.inner[2u].col0, v.inner[2u].col1, v.inner[2u].col2, v.inner[2u].col3));
  float16_t l = length(v.inner[0u].col1.zxy);
  float16_t a = abs(v.inner[0u].col1.zxy.x);
  float16_t v_2 = (t[0u].x + float16_t(l));
  v_1.inner = (v_2 + float16_t(a));
}
