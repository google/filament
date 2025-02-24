#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct mat4x3_f16_std140 {
  f16vec3 col0;
  f16vec3 col1;
  f16vec3 col2;
  f16vec3 col3;
};

layout(binding = 0, std140)
uniform a_block_std140_1_ubo {
  mat4x3_f16_std140 inner[4];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  float16_t inner;
} v_1;
int counter = 0;
int i() {
  counter = (counter + 1);
  return counter;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_2 = min(uint(i()), 3u);
  f16mat4x3 v_3 = f16mat4x3(v.inner[v_2].col0, v.inner[v_2].col1, v.inner[v_2].col2, v.inner[v_2].col3);
  f16vec3 v_4 = v_3[min(uint(i()), 3u)];
  mat4x3_f16_std140 v_5[4] = v.inner;
  f16mat4x3 v_6[4] = f16mat4x3[4](f16mat4x3(f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf)), f16mat4x3(f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf)), f16mat4x3(f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf)), f16mat4x3(f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf)));
  {
    uint v_7 = 0u;
    v_7 = 0u;
    while(true) {
      uint v_8 = v_7;
      if ((v_8 >= 4u)) {
        break;
      }
      v_6[v_8] = f16mat4x3(v_5[v_8].col0, v_5[v_8].col1, v_5[v_8].col2, v_5[v_8].col3);
      {
        v_7 = (v_8 + 1u);
      }
      continue;
    }
  }
  f16mat4x3 l_a[4] = v_6;
  f16mat4x3 l_a_i = v_3;
  f16vec3 l_a_i_i = v_4;
  v_1.inner = (((v_4.x + l_a[0u][0u].x) + l_a_i[0u].x) + l_a_i_i.x);
}
