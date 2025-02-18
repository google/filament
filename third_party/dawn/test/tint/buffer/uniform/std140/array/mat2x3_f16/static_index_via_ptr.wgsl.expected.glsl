#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct mat2x3_f16_std140 {
  f16vec3 col0;
  f16vec3 col1;
};

layout(binding = 0, std140)
uniform a_block_std140_1_ubo {
  mat2x3_f16_std140 inner[4];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  float16_t inner;
} v_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f16mat2x3 v_2 = f16mat2x3(v.inner[2u].col0, v.inner[2u].col1);
  mat2x3_f16_std140 v_3[4] = v.inner;
  f16mat2x3 v_4[4] = f16mat2x3[4](f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf)), f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf)), f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf)), f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf)));
  {
    uint v_5 = 0u;
    v_5 = 0u;
    while(true) {
      uint v_6 = v_5;
      if ((v_6 >= 4u)) {
        break;
      }
      v_4[v_6] = f16mat2x3(v_3[v_6].col0, v_3[v_6].col1);
      {
        v_5 = (v_6 + 1u);
      }
      continue;
    }
  }
  f16mat2x3 l_a[4] = v_4;
  f16mat2x3 l_a_i = v_2;
  f16vec3 l_a_i_i = v_2[1u];
  v_1.inner = (((v_2[1u].x + l_a[0u][0u].x) + l_a_i[0u].x) + l_a_i_i.x);
}
