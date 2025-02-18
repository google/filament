#version 310 es


struct TintTextureUniformData {
  uint tint_builtin_value_0;
};

layout(binding = 0, std140)
uniform tint_symbol_1_ubo {
  TintTextureUniformData inner;
} v;
layout(local_size_x = 6, local_size_y = 1, local_size_z = 1) in;
void main() {
  {
    uvec2 tint_loop_idx = uvec2(0u);
    uint level = v.inner.tint_builtin_value_0;
    while(true) {
      if (all(equal(tint_loop_idx, uvec2(4294967295u)))) {
        break;
      }
      if ((level > 0u)) {
      } else {
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
