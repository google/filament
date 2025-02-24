//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, rgba16ui) uniform highp writeonly uimage2DArray arg_0;
void textureStore_33cec0() {
  imageStore(arg_0, ivec3(ivec2(uvec2(1u)), 1), uvec4(1u));
}
void main() {
  textureStore_33cec0();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, rgba16ui) uniform highp writeonly uimage2DArray arg_0;
void textureStore_33cec0() {
  imageStore(arg_0, ivec3(ivec2(uvec2(1u)), 1), uvec4(1u));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  textureStore_33cec0();
}
