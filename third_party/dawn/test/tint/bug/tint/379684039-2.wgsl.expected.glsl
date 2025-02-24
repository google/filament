#version 310 es


struct FSUniformData {
  vec4 k[7];
  ivec2 size;
  uint tint_pad_0;
  uint tint_pad_1;
};

uint idx = 0u;
layout(binding = 2, std430)
buffer FSUniforms_1_ssbo {
  FSUniformData fsUniformData[];
} _storage;
void v() {
  ivec2 vec = ivec2(0);
  {
    uvec2 tint_loop_idx = uvec2(0u);
    while(true) {
      if (all(equal(tint_loop_idx, uvec2(4294967295u)))) {
        break;
      }
      int v_1 = vec.y;
      uint v_2 = idx;
      uint v_3 = min(v_2, (uint(_storage.fsUniformData.length()) - 1u));
      if ((v_1 >= _storage.fsUniformData[v_3].size.y)) {
        break;
      }
      {
        uint tint_low_inc = (tint_loop_idx.x + 1u);
        tint_loop_idx.x = tint_low_inc;
        uint tint_carry = uint((tint_low_inc == 0u));
        tint_loop_idx.y = (tint_loop_idx.y + tint_carry);
      }
      continue;
    }
  }
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
