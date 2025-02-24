//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  uvec3 inner;
} v;
uvec3 firstTrailingBit_cb51ce() {
  uvec3 arg_0 = uvec3(1u);
  uvec3 v_1 = arg_0;
  uvec3 res = mix((mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u))) | (mix(uvec3(0u), uvec3(8u), equal(((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u))) | (mix(uvec3(0u), uvec3(4u), equal((((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) & uvec3(15u)), uvec3(0u))) | (mix(uvec3(0u), uvec3(2u), equal(((((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(4u), equal((((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) & uvec3(15u)), uvec3(0u)))) & uvec3(3u)), uvec3(0u))) | mix(uvec3(0u), uvec3(1u), equal((((((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(4u), equal((((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) & uvec3(15u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(2u), equal(((((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(4u), equal((((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) & uvec3(15u)), uvec3(0u)))) & uvec3(3u)), uvec3(0u)))) & uvec3(1u)), uvec3(0u))))))), uvec3(4294967295u), equal(((((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(4u), equal((((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) & uvec3(15u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(2u), equal(((((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(4u), equal((((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) & uvec3(15u)), uvec3(0u)))) & uvec3(3u)), uvec3(0u)))), uvec3(0u)));
  return res;
}
void main() {
  v.inner = firstTrailingBit_cb51ce();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uvec3 inner;
} v;
uvec3 firstTrailingBit_cb51ce() {
  uvec3 arg_0 = uvec3(1u);
  uvec3 v_1 = arg_0;
  uvec3 res = mix((mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u))) | (mix(uvec3(0u), uvec3(8u), equal(((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u))) | (mix(uvec3(0u), uvec3(4u), equal((((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) & uvec3(15u)), uvec3(0u))) | (mix(uvec3(0u), uvec3(2u), equal(((((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(4u), equal((((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) & uvec3(15u)), uvec3(0u)))) & uvec3(3u)), uvec3(0u))) | mix(uvec3(0u), uvec3(1u), equal((((((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(4u), equal((((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) & uvec3(15u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(2u), equal(((((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(4u), equal((((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) & uvec3(15u)), uvec3(0u)))) & uvec3(3u)), uvec3(0u)))) & uvec3(1u)), uvec3(0u))))))), uvec3(4294967295u), equal(((((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(4u), equal((((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) & uvec3(15u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(2u), equal(((((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(4u), equal((((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) & uvec3(15u)), uvec3(0u)))) & uvec3(3u)), uvec3(0u)))), uvec3(0u)));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = firstTrailingBit_cb51ce();
}
//
// vertex_main
//
#version 310 es


struct VertexOutput {
  vec4 pos;
  uvec3 prevent_dce;
};

layout(location = 0) flat out uvec3 tint_interstage_location0;
uvec3 firstTrailingBit_cb51ce() {
  uvec3 arg_0 = uvec3(1u);
  uvec3 v = arg_0;
  uvec3 res = mix((mix(uvec3(0u), uvec3(16u), equal((v & uvec3(65535u)), uvec3(0u))) | (mix(uvec3(0u), uvec3(8u), equal(((v >> mix(uvec3(0u), uvec3(16u), equal((v & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u))) | (mix(uvec3(0u), uvec3(4u), equal((((v >> mix(uvec3(0u), uvec3(16u), equal((v & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v >> mix(uvec3(0u), uvec3(16u), equal((v & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) & uvec3(15u)), uvec3(0u))) | (mix(uvec3(0u), uvec3(2u), equal(((((v >> mix(uvec3(0u), uvec3(16u), equal((v & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v >> mix(uvec3(0u), uvec3(16u), equal((v & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(4u), equal((((v >> mix(uvec3(0u), uvec3(16u), equal((v & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v >> mix(uvec3(0u), uvec3(16u), equal((v & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) & uvec3(15u)), uvec3(0u)))) & uvec3(3u)), uvec3(0u))) | mix(uvec3(0u), uvec3(1u), equal((((((v >> mix(uvec3(0u), uvec3(16u), equal((v & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v >> mix(uvec3(0u), uvec3(16u), equal((v & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(4u), equal((((v >> mix(uvec3(0u), uvec3(16u), equal((v & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v >> mix(uvec3(0u), uvec3(16u), equal((v & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) & uvec3(15u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(2u), equal(((((v >> mix(uvec3(0u), uvec3(16u), equal((v & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v >> mix(uvec3(0u), uvec3(16u), equal((v & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(4u), equal((((v >> mix(uvec3(0u), uvec3(16u), equal((v & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v >> mix(uvec3(0u), uvec3(16u), equal((v & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) & uvec3(15u)), uvec3(0u)))) & uvec3(3u)), uvec3(0u)))) & uvec3(1u)), uvec3(0u))))))), uvec3(4294967295u), equal(((((v >> mix(uvec3(0u), uvec3(16u), equal((v & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v >> mix(uvec3(0u), uvec3(16u), equal((v & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(4u), equal((((v >> mix(uvec3(0u), uvec3(16u), equal((v & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v >> mix(uvec3(0u), uvec3(16u), equal((v & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) & uvec3(15u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(2u), equal(((((v >> mix(uvec3(0u), uvec3(16u), equal((v & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v >> mix(uvec3(0u), uvec3(16u), equal((v & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(4u), equal((((v >> mix(uvec3(0u), uvec3(16u), equal((v & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v >> mix(uvec3(0u), uvec3(16u), equal((v & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) & uvec3(15u)), uvec3(0u)))) & uvec3(3u)), uvec3(0u)))), uvec3(0u)));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_1 = VertexOutput(vec4(0.0f), uvec3(0u));
  v_1.pos = vec4(0.0f);
  v_1.prevent_dce = firstTrailingBit_cb51ce();
  return v_1;
}
void main() {
  VertexOutput v_2 = vertex_main_inner();
  gl_Position = vec4(v_2.pos.x, -(v_2.pos.y), ((2.0f * v_2.pos.z) - v_2.pos.w), v_2.pos.w);
  tint_interstage_location0 = v_2.prevent_dce;
  gl_PointSize = 1.0f;
}
