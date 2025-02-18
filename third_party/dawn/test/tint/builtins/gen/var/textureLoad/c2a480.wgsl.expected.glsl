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
  ivec4 inner;
} v;
layout(binding = 0, std140)
uniform f_tint_symbol_ubo {
  TintTextureUniformData inner;
} v_1;
uniform highp isampler2D arg_0;
ivec4 textureLoad_c2a480() {
  ivec2 arg_1 = ivec2(1);
  int arg_2 = 1;
  ivec2 v_2 = arg_1;
  uint v_3 = (v_1.inner.tint_builtin_value_0 - 1u);
  uint v_4 = min(uint(arg_2), v_3);
  uvec2 v_5 = (uvec2(textureSize(arg_0, int(v_4))) - uvec2(1u));
  ivec2 v_6 = ivec2(min(uvec2(v_2), v_5));
  ivec4 res = texelFetch(arg_0, v_6, int(v_4));
  return res;
}
void main() {
  v.inner = textureLoad_c2a480();
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
  ivec4 inner;
} v;
layout(binding = 0, std140)
uniform tint_symbol_1_ubo {
  TintTextureUniformData inner;
} v_1;
uniform highp isampler2D arg_0;
ivec4 textureLoad_c2a480() {
  ivec2 arg_1 = ivec2(1);
  int arg_2 = 1;
  ivec2 v_2 = arg_1;
  uint v_3 = (v_1.inner.tint_builtin_value_0 - 1u);
  uint v_4 = min(uint(arg_2), v_3);
  uvec2 v_5 = (uvec2(textureSize(arg_0, int(v_4))) - uvec2(1u));
  ivec2 v_6 = ivec2(min(uvec2(v_2), v_5));
  ivec4 res = texelFetch(arg_0, v_6, int(v_4));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureLoad_c2a480();
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
  ivec4 prevent_dce;
};

layout(binding = 0, std140)
uniform v_tint_symbol_ubo {
  TintTextureUniformData inner;
} v;
uniform highp isampler2D arg_0;
layout(location = 0) flat out ivec4 tint_interstage_location0;
ivec4 textureLoad_c2a480() {
  ivec2 arg_1 = ivec2(1);
  int arg_2 = 1;
  ivec2 v_1 = arg_1;
  uint v_2 = (v.inner.tint_builtin_value_0 - 1u);
  uint v_3 = min(uint(arg_2), v_2);
  uvec2 v_4 = (uvec2(textureSize(arg_0, int(v_3))) - uvec2(1u));
  ivec2 v_5 = ivec2(min(uvec2(v_1), v_4));
  ivec4 res = texelFetch(arg_0, v_5, int(v_3));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_6 = VertexOutput(vec4(0.0f), ivec4(0));
  v_6.pos = vec4(0.0f);
  v_6.prevent_dce = textureLoad_c2a480();
  return v_6;
}
void main() {
  VertexOutput v_7 = vertex_main_inner();
  gl_Position = vec4(v_7.pos.x, -(v_7.pos.y), ((2.0f * v_7.pos.z) - v_7.pos.w), v_7.pos.w);
  tint_interstage_location0 = v_7.prevent_dce;
  gl_PointSize = 1.0f;
}
