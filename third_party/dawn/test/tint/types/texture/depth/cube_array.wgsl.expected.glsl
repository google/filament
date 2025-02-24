#version 460


struct TintTextureUniformData {
  uint tint_builtin_value_0;
};

layout(binding = 0, std140)
uniform tint_symbol_1_ubo {
  TintTextureUniformData inner;
} v;
uniform highp samplerCubeArray t_f;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_1 = (v.inner.tint_builtin_value_0 - 1u);
  uvec2 dims = uvec2(textureSize(t_f, int(min(uint(0), v_1))).xy);
}
