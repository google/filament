//
// fragment_main
//
#version 460
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  ivec4 inner;
} v;
uniform highp isamplerCubeArray arg_1_arg_2;
ivec4 textureGather_c0640c() {
  ivec4 res = textureGather(arg_1_arg_2, vec4(vec3(1.0f), float(1)), 1);
  return res;
}
void main() {
  v.inner = textureGather_c0640c();
}
//
// compute_main
//
#version 460

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  ivec4 inner;
} v;
uniform highp isamplerCubeArray arg_1_arg_2;
ivec4 textureGather_c0640c() {
  ivec4 res = textureGather(arg_1_arg_2, vec4(vec3(1.0f), float(1)), 1);
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureGather_c0640c();
}
//
// vertex_main
//
#version 460


struct VertexOutput {
  vec4 pos;
  ivec4 prevent_dce;
};

uniform highp isamplerCubeArray arg_1_arg_2;
layout(location = 0) flat out ivec4 tint_interstage_location0;
ivec4 textureGather_c0640c() {
  ivec4 res = textureGather(arg_1_arg_2, vec4(vec3(1.0f), float(1)), 1);
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v = VertexOutput(vec4(0.0f), ivec4(0));
  v.pos = vec4(0.0f);
  v.prevent_dce = textureGather_c0640c();
  return v;
}
void main() {
  VertexOutput v_1 = vertex_main_inner();
  gl_Position = vec4(v_1.pos.x, -(v_1.pos.y), ((2.0f * v_1.pos.z) - v_1.pos.w), v_1.pos.w);
  tint_interstage_location0 = v_1.prevent_dce;
  gl_PointSize = 1.0f;
}
