//
// fragment_main
//
#version 460
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  uvec4 inner;
} v;
uniform highp usamplerCubeArray arg_1_arg_2;
uvec4 textureGather_f2c6e3() {
  uvec4 res = textureGather(arg_1_arg_2, vec4(vec3(1.0f), float(1)), 1);
  return res;
}
void main() {
  v.inner = textureGather_f2c6e3();
}
//
// compute_main
//
#version 460

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uvec4 inner;
} v;
uniform highp usamplerCubeArray arg_1_arg_2;
uvec4 textureGather_f2c6e3() {
  uvec4 res = textureGather(arg_1_arg_2, vec4(vec3(1.0f), float(1)), 1);
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureGather_f2c6e3();
}
//
// vertex_main
//
#version 460


struct VertexOutput {
  vec4 pos;
  uvec4 prevent_dce;
};

uniform highp usamplerCubeArray arg_1_arg_2;
layout(location = 0) flat out uvec4 tint_interstage_location0;
uvec4 textureGather_f2c6e3() {
  uvec4 res = textureGather(arg_1_arg_2, vec4(vec3(1.0f), float(1)), 1);
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v = VertexOutput(vec4(0.0f), uvec4(0u));
  v.pos = vec4(0.0f);
  v.prevent_dce = textureGather_f2c6e3();
  return v;
}
void main() {
  VertexOutput v_1 = vertex_main_inner();
  gl_Position = vec4(v_1.pos.x, -(v_1.pos.y), ((2.0f * v_1.pos.z) - v_1.pos.w), v_1.pos.w);
  tint_interstage_location0 = v_1.prevent_dce;
  gl_PointSize = 1.0f;
}
