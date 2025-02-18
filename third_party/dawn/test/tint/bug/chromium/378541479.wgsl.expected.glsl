#version 310 es


struct TintTextureUniformData {
  uint tint_builtin_value_0;
};

layout(binding = 0, std140)
uniform level_block_1_ubo {
  uint inner;
} v;
layout(binding = 1, std140)
uniform coords_block_1_ubo {
  uvec2 inner;
} v_1;
layout(binding = 0, std140)
uniform tint_symbol_1_ubo {
  TintTextureUniformData inner;
} v_2;
uniform highp sampler2D tex;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uvec2 v_3 = v_1.inner;
  uint v_4 = min(v.inner, (v_2.inner.tint_builtin_value_0 - 1u));
  ivec2 v_5 = ivec2(min(v_3, (uvec2(textureSize(tex, int(v_4))) - uvec2(1u))));
  float res = texelFetch(tex, v_5, int(v_4)).x;
}
