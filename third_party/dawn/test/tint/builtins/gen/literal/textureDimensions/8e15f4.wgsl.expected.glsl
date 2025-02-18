//
// fragment_main
//
#version 460
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  uvec2 inner;
} v;
layout(binding = 0, rg32ui) uniform highp readonly uimage2D arg_0;
uvec2 textureDimensions_8e15f4() {
  uvec2 res = uvec2(imageSize(arg_0));
  return res;
}
void main() {
  v.inner = textureDimensions_8e15f4();
}
//
// compute_main
//
#version 460

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uvec2 inner;
} v;
layout(binding = 0, rg32ui) uniform highp readonly uimage2D arg_0;
uvec2 textureDimensions_8e15f4() {
  uvec2 res = uvec2(imageSize(arg_0));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureDimensions_8e15f4();
}
//
// vertex_main
//
#version 460


struct VertexOutput {
  vec4 pos;
  uvec2 prevent_dce;
};

layout(binding = 0, rg32ui) uniform highp readonly uimage2D arg_0;
layout(location = 0) flat out uvec2 tint_interstage_location0;
uvec2 textureDimensions_8e15f4() {
  uvec2 res = uvec2(imageSize(arg_0));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v = VertexOutput(vec4(0.0f), uvec2(0u));
  v.pos = vec4(0.0f);
  v.prevent_dce = textureDimensions_8e15f4();
  return v;
}
void main() {
  VertexOutput v_1 = vertex_main_inner();
  gl_Position = vec4(v_1.pos.x, -(v_1.pos.y), ((2.0f * v_1.pos.z) - v_1.pos.w), v_1.pos.w);
  tint_interstage_location0 = v_1.prevent_dce;
  gl_PointSize = 1.0f;
}
