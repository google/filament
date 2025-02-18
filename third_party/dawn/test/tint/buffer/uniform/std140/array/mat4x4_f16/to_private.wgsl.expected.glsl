#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct mat4x4_f16_std140 {
  f16vec4 col0;
  f16vec4 col1;
  f16vec4 col2;
  f16vec4 col3;
};

layout(binding = 0, std140)
uniform u_block_std140_1_ubo {
  mat4x4_f16_std140 inner[4];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  float16_t inner;
} v_1;
f16mat4 p[4] = f16mat4[4](f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)), f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)), f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)), f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)));
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat4x4_f16_std140 v_2[4] = v.inner;
  f16mat4 v_3[4] = f16mat4[4](f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)), f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)), f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)), f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)));
  {
    uint v_4 = 0u;
    v_4 = 0u;
    while(true) {
      uint v_5 = v_4;
      if ((v_5 >= 4u)) {
        break;
      }
      v_3[v_5] = f16mat4(v_2[v_5].col0, v_2[v_5].col1, v_2[v_5].col2, v_2[v_5].col3);
      {
        v_4 = (v_5 + 1u);
      }
      continue;
    }
  }
  p = v_3;
  p[1u] = f16mat4(v.inner[2u].col0, v.inner[2u].col1, v.inner[2u].col2, v.inner[2u].col3);
  p[1u][0u] = v.inner[0u].col1.ywxz;
  p[1u][0u].x = v.inner[0u].col1.x;
  v_1.inner = p[1u][0u].x;
}
