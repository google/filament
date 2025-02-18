//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  uint inner;
} v;
uint firstLeadingBit_f0779d() {
  uint arg_0 = 1u;
  uint v_1 = arg_0;
  uint res = mix((mix(16u, 0u, ((v_1 & 4294901760u) == 0u)) | (mix(8u, 0u, (((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) & 65280u) == 0u)) | (mix(4u, 0u, ((((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) >> mix(8u, 0u, (((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) & 65280u) == 0u))) & 240u) == 0u)) | (mix(2u, 0u, (((((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) >> mix(8u, 0u, (((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) & 65280u) == 0u))) >> mix(4u, 0u, ((((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) >> mix(8u, 0u, (((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) & 65280u) == 0u))) & 240u) == 0u))) & 12u) == 0u)) | mix(1u, 0u, ((((((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) >> mix(8u, 0u, (((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) & 65280u) == 0u))) >> mix(4u, 0u, ((((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) >> mix(8u, 0u, (((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) & 65280u) == 0u))) & 240u) == 0u))) >> mix(2u, 0u, (((((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) >> mix(8u, 0u, (((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) & 65280u) == 0u))) >> mix(4u, 0u, ((((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) >> mix(8u, 0u, (((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) & 65280u) == 0u))) & 240u) == 0u))) & 12u) == 0u))) & 2u) == 0u)))))), 4294967295u, (((((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) >> mix(8u, 0u, (((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) & 65280u) == 0u))) >> mix(4u, 0u, ((((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) >> mix(8u, 0u, (((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) & 65280u) == 0u))) & 240u) == 0u))) >> mix(2u, 0u, (((((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) >> mix(8u, 0u, (((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) & 65280u) == 0u))) >> mix(4u, 0u, ((((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) >> mix(8u, 0u, (((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) & 65280u) == 0u))) & 240u) == 0u))) & 12u) == 0u))) == 0u));
  return res;
}
void main() {
  v.inner = firstLeadingBit_f0779d();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uint inner;
} v;
uint firstLeadingBit_f0779d() {
  uint arg_0 = 1u;
  uint v_1 = arg_0;
  uint res = mix((mix(16u, 0u, ((v_1 & 4294901760u) == 0u)) | (mix(8u, 0u, (((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) & 65280u) == 0u)) | (mix(4u, 0u, ((((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) >> mix(8u, 0u, (((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) & 65280u) == 0u))) & 240u) == 0u)) | (mix(2u, 0u, (((((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) >> mix(8u, 0u, (((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) & 65280u) == 0u))) >> mix(4u, 0u, ((((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) >> mix(8u, 0u, (((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) & 65280u) == 0u))) & 240u) == 0u))) & 12u) == 0u)) | mix(1u, 0u, ((((((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) >> mix(8u, 0u, (((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) & 65280u) == 0u))) >> mix(4u, 0u, ((((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) >> mix(8u, 0u, (((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) & 65280u) == 0u))) & 240u) == 0u))) >> mix(2u, 0u, (((((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) >> mix(8u, 0u, (((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) & 65280u) == 0u))) >> mix(4u, 0u, ((((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) >> mix(8u, 0u, (((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) & 65280u) == 0u))) & 240u) == 0u))) & 12u) == 0u))) & 2u) == 0u)))))), 4294967295u, (((((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) >> mix(8u, 0u, (((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) & 65280u) == 0u))) >> mix(4u, 0u, ((((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) >> mix(8u, 0u, (((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) & 65280u) == 0u))) & 240u) == 0u))) >> mix(2u, 0u, (((((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) >> mix(8u, 0u, (((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) & 65280u) == 0u))) >> mix(4u, 0u, ((((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) >> mix(8u, 0u, (((v_1 >> mix(16u, 0u, ((v_1 & 4294901760u) == 0u))) & 65280u) == 0u))) & 240u) == 0u))) & 12u) == 0u))) == 0u));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = firstLeadingBit_f0779d();
}
//
// vertex_main
//
#version 310 es


struct VertexOutput {
  vec4 pos;
  uint prevent_dce;
};

layout(location = 0) flat out uint tint_interstage_location0;
uint firstLeadingBit_f0779d() {
  uint arg_0 = 1u;
  uint v = arg_0;
  uint res = mix((mix(16u, 0u, ((v & 4294901760u) == 0u)) | (mix(8u, 0u, (((v >> mix(16u, 0u, ((v & 4294901760u) == 0u))) & 65280u) == 0u)) | (mix(4u, 0u, ((((v >> mix(16u, 0u, ((v & 4294901760u) == 0u))) >> mix(8u, 0u, (((v >> mix(16u, 0u, ((v & 4294901760u) == 0u))) & 65280u) == 0u))) & 240u) == 0u)) | (mix(2u, 0u, (((((v >> mix(16u, 0u, ((v & 4294901760u) == 0u))) >> mix(8u, 0u, (((v >> mix(16u, 0u, ((v & 4294901760u) == 0u))) & 65280u) == 0u))) >> mix(4u, 0u, ((((v >> mix(16u, 0u, ((v & 4294901760u) == 0u))) >> mix(8u, 0u, (((v >> mix(16u, 0u, ((v & 4294901760u) == 0u))) & 65280u) == 0u))) & 240u) == 0u))) & 12u) == 0u)) | mix(1u, 0u, ((((((v >> mix(16u, 0u, ((v & 4294901760u) == 0u))) >> mix(8u, 0u, (((v >> mix(16u, 0u, ((v & 4294901760u) == 0u))) & 65280u) == 0u))) >> mix(4u, 0u, ((((v >> mix(16u, 0u, ((v & 4294901760u) == 0u))) >> mix(8u, 0u, (((v >> mix(16u, 0u, ((v & 4294901760u) == 0u))) & 65280u) == 0u))) & 240u) == 0u))) >> mix(2u, 0u, (((((v >> mix(16u, 0u, ((v & 4294901760u) == 0u))) >> mix(8u, 0u, (((v >> mix(16u, 0u, ((v & 4294901760u) == 0u))) & 65280u) == 0u))) >> mix(4u, 0u, ((((v >> mix(16u, 0u, ((v & 4294901760u) == 0u))) >> mix(8u, 0u, (((v >> mix(16u, 0u, ((v & 4294901760u) == 0u))) & 65280u) == 0u))) & 240u) == 0u))) & 12u) == 0u))) & 2u) == 0u)))))), 4294967295u, (((((v >> mix(16u, 0u, ((v & 4294901760u) == 0u))) >> mix(8u, 0u, (((v >> mix(16u, 0u, ((v & 4294901760u) == 0u))) & 65280u) == 0u))) >> mix(4u, 0u, ((((v >> mix(16u, 0u, ((v & 4294901760u) == 0u))) >> mix(8u, 0u, (((v >> mix(16u, 0u, ((v & 4294901760u) == 0u))) & 65280u) == 0u))) & 240u) == 0u))) >> mix(2u, 0u, (((((v >> mix(16u, 0u, ((v & 4294901760u) == 0u))) >> mix(8u, 0u, (((v >> mix(16u, 0u, ((v & 4294901760u) == 0u))) & 65280u) == 0u))) >> mix(4u, 0u, ((((v >> mix(16u, 0u, ((v & 4294901760u) == 0u))) >> mix(8u, 0u, (((v >> mix(16u, 0u, ((v & 4294901760u) == 0u))) & 65280u) == 0u))) & 240u) == 0u))) & 12u) == 0u))) == 0u));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_1 = VertexOutput(vec4(0.0f), 0u);
  v_1.pos = vec4(0.0f);
  v_1.prevent_dce = firstLeadingBit_f0779d();
  return v_1;
}
void main() {
  VertexOutput v_2 = vertex_main_inner();
  gl_Position = vec4(v_2.pos.x, -(v_2.pos.y), ((2.0f * v_2.pos.z) - v_2.pos.w), v_2.pos.w);
  tint_interstage_location0 = v_2.prevent_dce;
  gl_PointSize = 1.0f;
}
