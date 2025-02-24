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
shared f16mat2x4 w[4];
void f_inner(uint tint_local_index) {
  {
    uint v_1 = 0u;
    v_1 = tint_local_index;
    while(true) {
      uint v_2 = v_1;
      if ((v_2 >= 4u)) {
        break;
      }
      w[v_2] = f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf));
      {
        v_1 = (v_2 + 1u);
      }
      continue;
    }
  }
  barrier();
  mat2x4_f16_std140 v_3[4] = v.inner;
  f16mat2x4 v_4[4] = f16mat2x4[4](f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf)), f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf)), f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf)), f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf)));
  {
    uint v_5 = 0u;
    v_5 = 0u;
    while(true) {
      uint v_6 = v_5;
      if ((v_6 >= 4u)) {
        break;
      }
      v_4[v_6] = f16mat2x4(v_3[v_6].col0, v_3[v_6].col1);
      {
        v_5 = (v_6 + 1u);
      }
      continue;
    }
  }
  w = v_4;
  w[1u] = f16mat2x4(v.inner[2u].col0, v.inner[2u].col1);
  w[1u][0u] = v.inner[0u].col1.ywxz;
  w[1u][0u].x = v.inner[0u].col1.x;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f_inner(gl_LocalInvocationIndex);
}
