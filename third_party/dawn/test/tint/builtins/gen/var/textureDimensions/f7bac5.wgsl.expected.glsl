//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  uvec2 inner;
} v;
layout(binding = 0, rgba32i) uniform highp readonly iimage2DArray arg_0;
uvec2 textureDimensions_f7bac5() {
  uvec2 res = uvec2(imageSize(arg_0).xy);
  return res;
}
void main() {
  v.inner = textureDimensions_f7bac5();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uvec2 inner;
} v;
layout(binding = 0, rgba32i) uniform highp readonly iimage2DArray arg_0;
uvec2 textureDimensions_f7bac5() {
  uvec2 res = uvec2(imageSize(arg_0).xy);
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureDimensions_f7bac5();
}
//
// vertex_main
//
#version 310 es


struct VertexOutput {
  vec4 pos;
  uvec2 prevent_dce;
};

layout(binding = 0, rgba32i) uniform highp readonly iimage2DArray arg_0;
layout(location = 0) flat out uvec2 tint_interstage_location0;
uvec2 textureDimensions_f7bac5() {
  uvec2 res = uvec2(imageSize(arg_0).xy);
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v = VertexOutput(vec4(0.0f), uvec2(0u));
  v.pos = vec4(0.0f);
  v.prevent_dce = textureDimensions_f7bac5();
  return v;
}
void main() {
  VertexOutput v_1 = vertex_main_inner();
  gl_Position = vec4(v_1.pos.x, -(v_1.pos.y), ((2.0f * v_1.pos.z) - v_1.pos.w), v_1.pos.w);
  tint_interstage_location0 = v_1.prevent_dce;
  gl_PointSize = 1.0f;
}
