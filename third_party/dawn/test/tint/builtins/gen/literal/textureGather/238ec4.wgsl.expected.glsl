//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  vec4 inner;
} v;
uniform highp sampler2DArray arg_1_arg_2;
vec4 textureGather_238ec4() {
  vec3 v_1 = vec3(vec2(1.0f), float(1u));
  vec4 res = textureGatherOffset(arg_1_arg_2, v_1, ivec2(1), int(1u));
  return res;
}
void main() {
  v.inner = textureGather_238ec4();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  vec4 inner;
} v;
uniform highp sampler2DArray arg_1_arg_2;
vec4 textureGather_238ec4() {
  vec3 v_1 = vec3(vec2(1.0f), float(1u));
  vec4 res = textureGatherOffset(arg_1_arg_2, v_1, ivec2(1), int(1u));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureGather_238ec4();
}
//
// vertex_main
//
#version 310 es


struct VertexOutput {
  vec4 pos;
  vec4 prevent_dce;
};

uniform highp sampler2DArray arg_1_arg_2;
layout(location = 0) flat out vec4 tint_interstage_location0;
vec4 textureGather_238ec4() {
  vec3 v = vec3(vec2(1.0f), float(1u));
  vec4 res = textureGatherOffset(arg_1_arg_2, v, ivec2(1), int(1u));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_1 = VertexOutput(vec4(0.0f), vec4(0.0f));
  v_1.pos = vec4(0.0f);
  v_1.prevent_dce = textureGather_238ec4();
  return v_1;
}
void main() {
  VertexOutput v_2 = vertex_main_inner();
  gl_Position = vec4(v_2.pos.x, -(v_2.pos.y), ((2.0f * v_2.pos.z) - v_2.pos.w), v_2.pos.w);
  tint_interstage_location0 = v_2.prevent_dce;
  gl_PointSize = 1.0f;
}
