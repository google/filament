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
layout(binding = 0, rgba16f) uniform highp writeonly image3D arg_0;
uvec3 textureDimensions_f3a2ac() {
  uvec3 res = uvec3(imageSize(arg_0));
  return res;
}
void main() {
  v.inner = textureDimensions_f3a2ac();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uvec3 inner;
} v;
layout(binding = 0, rgba16f) uniform highp writeonly image3D arg_0;
uvec3 textureDimensions_f3a2ac() {
  uvec3 res = uvec3(imageSize(arg_0));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureDimensions_f3a2ac();
}
