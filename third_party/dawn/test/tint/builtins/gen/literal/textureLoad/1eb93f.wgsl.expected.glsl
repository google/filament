//
// fragment_main
//
#version 460
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  vec4 inner;
} v;
layout(binding = 0, rg32f) uniform highp readonly image2D arg_0;
vec4 textureLoad_1eb93f() {
  vec4 res = imageLoad(arg_0, ivec2(min(uvec2(1u), (uvec2(imageSize(arg_0)) - uvec2(1u)))));
  return res;
}
void main() {
  v.inner = textureLoad_1eb93f();
}
//
// compute_main
//
#version 460

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  vec4 inner;
} v;
layout(binding = 0, rg32f) uniform highp readonly image2D arg_0;
vec4 textureLoad_1eb93f() {
  vec4 res = imageLoad(arg_0, ivec2(min(uvec2(1u), (uvec2(imageSize(arg_0)) - uvec2(1u)))));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureLoad_1eb93f();
}
//
// vertex_main
//
#version 460


struct VertexOutput {
  vec4 pos;
  vec4 prevent_dce;
};

layout(binding = 0, rg32f) uniform highp readonly image2D arg_0;
layout(location = 0) flat out vec4 tint_interstage_location0;
vec4 textureLoad_1eb93f() {
  vec4 res = imageLoad(arg_0, ivec2(min(uvec2(1u), (uvec2(imageSize(arg_0)) - uvec2(1u)))));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v = VertexOutput(vec4(0.0f), vec4(0.0f));
  v.pos = vec4(0.0f);
  v.prevent_dce = textureLoad_1eb93f();
  return v;
}
void main() {
  VertexOutput v_1 = vertex_main_inner();
  gl_Position = vec4(v_1.pos.x, -(v_1.pos.y), ((2.0f * v_1.pos.z) - v_1.pos.w), v_1.pos.w);
  tint_interstage_location0 = v_1.prevent_dce;
  gl_PointSize = 1.0f;
}
