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
uniform highp isampler2DArray arg_0;
ivec4 textureLoad_2363be() {
  uvec2 arg_1 = uvec2(1u);
  int arg_2 = 1;
  uint arg_3 = 1u;
  uvec2 v_2 = arg_1;
  int v_3 = arg_2;
  uint v_4 = arg_3;
  uint v_5 = (uint(textureSize(arg_0, 0).z) - 1u);
  uint v_6 = min(uint(v_3), v_5);
  uint v_7 = min(v_4, (v_1.inner.tint_builtin_value_0 - 1u));
  ivec2 v_8 = ivec2(min(v_2, (uvec2(textureSize(arg_0, int(v_7)).xy) - uvec2(1u))));
  ivec3 v_9 = ivec3(v_8, int(v_6));
  ivec4 res = texelFetch(arg_0, v_9, int(v_7));
  return res;
}
void main() {
  v.inner = textureLoad_2363be();
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
uniform highp isampler2DArray arg_0;
ivec4 textureLoad_2363be() {
  uvec2 arg_1 = uvec2(1u);
  int arg_2 = 1;
  uint arg_3 = 1u;
  uvec2 v_2 = arg_1;
  int v_3 = arg_2;
  uint v_4 = arg_3;
  uint v_5 = (uint(textureSize(arg_0, 0).z) - 1u);
  uint v_6 = min(uint(v_3), v_5);
  uint v_7 = min(v_4, (v_1.inner.tint_builtin_value_0 - 1u));
  ivec2 v_8 = ivec2(min(v_2, (uvec2(textureSize(arg_0, int(v_7)).xy) - uvec2(1u))));
  ivec3 v_9 = ivec3(v_8, int(v_6));
  ivec4 res = texelFetch(arg_0, v_9, int(v_7));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureLoad_2363be();
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
uniform highp isampler2DArray arg_0;
layout(location = 0) flat out ivec4 tint_interstage_location0;
ivec4 textureLoad_2363be() {
  uvec2 arg_1 = uvec2(1u);
  int arg_2 = 1;
  uint arg_3 = 1u;
  uvec2 v_1 = arg_1;
  int v_2 = arg_2;
  uint v_3 = arg_3;
  uint v_4 = (uint(textureSize(arg_0, 0).z) - 1u);
  uint v_5 = min(uint(v_2), v_4);
  uint v_6 = min(v_3, (v.inner.tint_builtin_value_0 - 1u));
  ivec2 v_7 = ivec2(min(v_1, (uvec2(textureSize(arg_0, int(v_6)).xy) - uvec2(1u))));
  ivec3 v_8 = ivec3(v_7, int(v_5));
  ivec4 res = texelFetch(arg_0, v_8, int(v_6));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_9 = VertexOutput(vec4(0.0f), ivec4(0));
  v_9.pos = vec4(0.0f);
  v_9.prevent_dce = textureLoad_2363be();
  return v_9;
}
void main() {
  VertexOutput v_10 = vertex_main_inner();
  gl_Position = vec4(v_10.pos.x, -(v_10.pos.y), ((2.0f * v_10.pos.z) - v_10.pos.w), v_10.pos.w);
  tint_interstage_location0 = v_10.prevent_dce;
  gl_PointSize = 1.0f;
}
