#version 310 es


struct TintTextureUniformData {
  uint tint_builtin_value_0;
  uint tint_builtin_value_1;
  uint tint_builtin_value_2;
};

layout(binding = 0, std140)
uniform tint_symbol_1_ubo {
  TintTextureUniformData inner;
} v;
uniform highp samplerCube t_f;
uniform highp isamplerCube t_i;
uniform highp usamplerCube t_u;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_1 = (v.inner.tint_builtin_value_0 - 1u);
  uvec2 fdims = uvec2(textureSize(t_f, int(min(uint(1), v_1))));
  uint v_2 = (v.inner.tint_builtin_value_1 - 1u);
  uvec2 idims = uvec2(textureSize(t_i, int(min(uint(1), v_2))));
  uint v_3 = (v.inner.tint_builtin_value_2 - 1u);
  uvec2 udims = uvec2(textureSize(t_u, int(min(uint(1), v_3))));
}
