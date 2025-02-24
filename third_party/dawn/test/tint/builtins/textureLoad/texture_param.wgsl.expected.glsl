//
// vertex_main
//
#version 310 es


struct TintTextureUniformData {
  uint tint_builtin_value_0;
};

layout(binding = 0, std140)
uniform v_tint_symbol_ubo {
  TintTextureUniformData inner;
} v;
uniform highp isampler2D arg_0;
ivec4 textureLoad2d(ivec2 coords, int level, uint tint_tex_value) {
  uint v_1 = min(uint(level), (tint_tex_value - 1u));
  uvec2 v_2 = (uvec2(textureSize(arg_0, int(v_1))) - uvec2(1u));
  ivec2 v_3 = ivec2(min(uvec2(coords), v_2));
  return texelFetch(arg_0, v_3, int(v_1));
}
void doTextureLoad() {
  ivec4 res = textureLoad2d(ivec2(0), 0, v.inner.tint_builtin_value_0);
}
vec4 vertex_main_inner() {
  doTextureLoad();
  return vec4(0.0f);
}
void main() {
  vec4 v_4 = vertex_main_inner();
  gl_Position = vec4(v_4.x, -(v_4.y), ((2.0f * v_4.z) - v_4.w), v_4.w);
  gl_PointSize = 1.0f;
}
//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;


struct TintTextureUniformData {
  uint tint_builtin_value_0;
};

layout(binding = 0, std140)
uniform f_tint_symbol_ubo {
  TintTextureUniformData inner;
} v;
uniform highp isampler2D arg_0;
ivec4 textureLoad2d(ivec2 coords, int level, uint tint_tex_value) {
  uint v_1 = min(uint(level), (tint_tex_value - 1u));
  uvec2 v_2 = (uvec2(textureSize(arg_0, int(v_1))) - uvec2(1u));
  ivec2 v_3 = ivec2(min(uvec2(coords), v_2));
  return texelFetch(arg_0, v_3, int(v_1));
}
void doTextureLoad() {
  ivec4 res = textureLoad2d(ivec2(0), 0, v.inner.tint_builtin_value_0);
}
void main() {
  doTextureLoad();
}
//
// compute_main
//
#version 310 es


struct TintTextureUniformData {
  uint tint_builtin_value_0;
};

layout(binding = 0, std140)
uniform tint_symbol_1_ubo {
  TintTextureUniformData inner;
} v;
uniform highp isampler2D arg_0;
ivec4 textureLoad2d(ivec2 coords, int level, uint tint_tex_value) {
  uint v_1 = min(uint(level), (tint_tex_value - 1u));
  uvec2 v_2 = (uvec2(textureSize(arg_0, int(v_1))) - uvec2(1u));
  ivec2 v_3 = ivec2(min(uvec2(coords), v_2));
  return texelFetch(arg_0, v_3, int(v_1));
}
void doTextureLoad() {
  ivec4 res = textureLoad2d(ivec2(0), 0, v.inner.tint_builtin_value_0);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  doTextureLoad();
}
