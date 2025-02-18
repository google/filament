//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;


struct TintTextureUniformData {
  uint tint_builtin_value_0;
};

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  uint inner;
} v;
layout(binding = 0, std140)
uniform f_tint_symbol_ubo {
  TintTextureUniformData inner;
} v_1;
uniform highp usampler2D arg_0;
uint textureDimensions_920006() {
  int arg_1 = 1;
  uint v_2 = (v_1.inner.tint_builtin_value_0 - 1u);
  uint res = uvec2(textureSize(arg_0, int(min(uint(arg_1), v_2)))).x;
  return res;
}
void main() {
  v.inner = textureDimensions_920006();
}
//
// compute_main
//
#version 310 es


struct TintTextureUniformData {
  uint tint_builtin_value_0;
};

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uint inner;
} v;
layout(binding = 0, std140)
uniform tint_symbol_1_ubo {
  TintTextureUniformData inner;
} v_1;
uniform highp usampler2D arg_0;
uint textureDimensions_920006() {
  int arg_1 = 1;
  uint v_2 = (v_1.inner.tint_builtin_value_0 - 1u);
  uint res = uvec2(textureSize(arg_0, int(min(uint(arg_1), v_2)))).x;
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureDimensions_920006();
}
//
// vertex_main
//
#version 310 es


struct TintTextureUniformData {
  uint tint_builtin_value_0;
};

struct VertexOutput {
  vec4 pos;
  uint prevent_dce;
};

layout(binding = 0, std140)
uniform v_tint_symbol_ubo {
  TintTextureUniformData inner;
} v;
uniform highp usampler2D arg_0;
layout(location = 0) flat out uint tint_interstage_location0;
uint textureDimensions_920006() {
  int arg_1 = 1;
  uint v_1 = (v.inner.tint_builtin_value_0 - 1u);
  uint res = uvec2(textureSize(arg_0, int(min(uint(arg_1), v_1)))).x;
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_2 = VertexOutput(vec4(0.0f), 0u);
  v_2.pos = vec4(0.0f);
  v_2.prevent_dce = textureDimensions_920006();
  return v_2;
}
void main() {
  VertexOutput v_3 = vertex_main_inner();
  gl_Position = vec4(v_3.pos.x, -(v_3.pos.y), ((2.0f * v_3.pos.z) - v_3.pos.w), v_3.pos.w);
  tint_interstage_location0 = v_3.prevent_dce;
  gl_PointSize = 1.0f;
}
