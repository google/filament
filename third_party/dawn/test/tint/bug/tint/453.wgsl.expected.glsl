#version 310 es


struct TintTextureUniformData {
  uint tint_builtin_value_0;
  uint tint_builtin_value_1;
};

layout(binding = 1, r32ui) uniform highp writeonly uimage2D Dst;
layout(binding = 0, std140)
uniform tint_symbol_1_ubo {
  TintTextureUniformData inner;
} v;
uniform highp usampler2D Src;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uvec4 srcValue = uvec4(0u);
  uint v_1 = (v.inner.tint_builtin_value_0 - 1u);
  uint v_2 = min(uint(0), v_1);
  uvec2 v_3 = (uvec2(textureSize(Src, int(v_2))) - uvec2(1u));
  ivec2 v_4 = ivec2(min(uvec2(ivec2(0)), v_3));
  uvec4 x_22 = texelFetch(Src, v_4, int(v_2));
  srcValue = x_22;
  uint x_24 = srcValue.x;
  uint x_25 = (x_24 + 1u);
  uvec4 x_27 = srcValue;
  imageStore(Dst, ivec2(0), x_27.xxxx);
}
