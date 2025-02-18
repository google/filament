#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct mat2x3_f16_std140 {
  f16vec3 col0;
  f16vec3 col1;
};

layout(binding = 0, std140)
uniform u_block_std140_1_ubo {
  mat2x3_f16_std140 inner[4];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  float16_t inner;
} v_1;
shared f16mat2x3 w[4];
void f_inner(uint tint_local_index) {
  {
    uint v_2 = 0u;
    v_2 = tint_local_index;
    while(true) {
      uint v_3 = v_2;
      if ((v_3 >= 4u)) {
        break;
      }
      w[v_3] = f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf));
      {
        v_2 = (v_3 + 1u);
      }
      continue;
    }
  }
  barrier();
  mat2x3_f16_std140 v_4[4] = v.inner;
  f16mat2x3 v_5[4] = f16mat2x3[4](f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf)), f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf)), f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf)), f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf)));
  {
    uint v_6 = 0u;
    v_6 = 0u;
    while(true) {
      uint v_7 = v_6;
      if ((v_7 >= 4u)) {
        break;
      }
      v_5[v_7] = f16mat2x3(v_4[v_7].col0, v_4[v_7].col1);
      {
        v_6 = (v_7 + 1u);
      }
      continue;
    }
  }
  w = v_5;
  w[1u] = f16mat2x3(v.inner[2u].col0, v.inner[2u].col1);
  w[1u][0u] = v.inner[0u].col1.zxy;
  w[1u][0u].x = v.inner[0u].col1.x;
  v_1.inner = w[1u][0u].x;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f_inner(gl_LocalInvocationIndex);
}
