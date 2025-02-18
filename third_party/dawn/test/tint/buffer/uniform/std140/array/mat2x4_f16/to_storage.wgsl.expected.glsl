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
  f16mat2x4 inner[4];
} v_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat2x4_f16_std140 v_2[4] = v.inner;
  f16mat2x4 v_3[4] = f16mat2x4[4](f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf)), f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf)), f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf)), f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf)));
  {
    uint v_4 = 0u;
    v_4 = 0u;
    while(true) {
      uint v_5 = v_4;
      if ((v_5 >= 4u)) {
        break;
      }
      v_3[v_5] = f16mat2x4(v_2[v_5].col0, v_2[v_5].col1);
      {
        v_4 = (v_5 + 1u);
      }
      continue;
    }
  }
  v_1.inner = v_3;
  v_1.inner[1u] = f16mat2x4(v.inner[2u].col0, v.inner[2u].col1);
  v_1.inner[1u][0u] = v.inner[0u].col1.ywxz;
  v_1.inner[1u][0u].x = v.inner[0u].col1.x;
}
