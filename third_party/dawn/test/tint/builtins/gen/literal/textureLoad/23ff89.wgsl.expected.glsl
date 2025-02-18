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
layout(binding = 0, rgba16ui) uniform highp readonly uimage2DArray arg_0;
uvec4 textureLoad_23ff89() {
  uint v_1 = min(1u, (uint(imageSize(arg_0).z) - 1u));
  ivec2 v_2 = ivec2(min(uvec2(1u), (uvec2(imageSize(arg_0).xy) - uvec2(1u))));
  uvec4 res = imageLoad(arg_0, ivec3(v_2, int(v_1)));
  return res;
}
void main() {
  v.inner = textureLoad_23ff89();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uvec4 inner;
} v;
layout(binding = 0, rgba16ui) uniform highp readonly uimage2DArray arg_0;
uvec4 textureLoad_23ff89() {
  uint v_1 = min(1u, (uint(imageSize(arg_0).z) - 1u));
  ivec2 v_2 = ivec2(min(uvec2(1u), (uvec2(imageSize(arg_0).xy) - uvec2(1u))));
  uvec4 res = imageLoad(arg_0, ivec3(v_2, int(v_1)));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureLoad_23ff89();
}
//
// vertex_main
//
#version 310 es


struct VertexOutput {
  vec4 pos;
  uvec4 prevent_dce;
};

layout(binding = 0, rgba16ui) uniform highp readonly uimage2DArray arg_0;
layout(location = 0) flat out uvec4 tint_interstage_location0;
uvec4 textureLoad_23ff89() {
  uint v = min(1u, (uint(imageSize(arg_0).z) - 1u));
  ivec2 v_1 = ivec2(min(uvec2(1u), (uvec2(imageSize(arg_0).xy) - uvec2(1u))));
  uvec4 res = imageLoad(arg_0, ivec3(v_1, int(v)));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_2 = VertexOutput(vec4(0.0f), uvec4(0u));
  v_2.pos = vec4(0.0f);
  v_2.prevent_dce = textureLoad_23ff89();
  return v_2;
}
void main() {
  VertexOutput v_3 = vertex_main_inner();
  gl_Position = vec4(v_3.pos.x, -(v_3.pos.y), ((2.0f * v_3.pos.z) - v_3.pos.w), v_3.pos.w);
  tint_interstage_location0 = v_3.prevent_dce;
  gl_PointSize = 1.0f;
}
