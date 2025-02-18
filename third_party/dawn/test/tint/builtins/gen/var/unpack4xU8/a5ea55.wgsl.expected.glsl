//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  uvec4 inner;
} v;
uvec4 unpack4xU8_a5ea55() {
  uint arg_0 = 1u;
  uint v_1 = arg_0;
  uvec4 v_2 = uvec4(0u, 8u, 16u, 24u);
  uvec4 v_3 = (uvec4(v_1) >> v_2);
  uvec4 res = (v_3 & uvec4(255u));
  return res;
}
void main() {
  v.inner = unpack4xU8_a5ea55();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uvec4 inner;
} v;
uvec4 unpack4xU8_a5ea55() {
  uint arg_0 = 1u;
  uint v_1 = arg_0;
  uvec4 v_2 = uvec4(0u, 8u, 16u, 24u);
  uvec4 v_3 = (uvec4(v_1) >> v_2);
  uvec4 res = (v_3 & uvec4(255u));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = unpack4xU8_a5ea55();
}
//
// vertex_main
//
#version 310 es


struct VertexOutput {
  vec4 pos;
  uvec4 prevent_dce;
};

layout(location = 0) flat out uvec4 tint_interstage_location0;
uvec4 unpack4xU8_a5ea55() {
  uint arg_0 = 1u;
  uint v = arg_0;
  uvec4 v_1 = uvec4(0u, 8u, 16u, 24u);
  uvec4 v_2 = (uvec4(v) >> v_1);
  uvec4 res = (v_2 & uvec4(255u));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_3 = VertexOutput(vec4(0.0f), uvec4(0u));
  v_3.pos = vec4(0.0f);
  v_3.prevent_dce = unpack4xU8_a5ea55();
  return v_3;
}
void main() {
  VertexOutput v_4 = vertex_main_inner();
  gl_Position = vec4(v_4.pos.x, -(v_4.pos.y), ((2.0f * v_4.pos.z) - v_4.pos.w), v_4.pos.w);
  tint_interstage_location0 = v_4.prevent_dce;
  gl_PointSize = 1.0f;
}
