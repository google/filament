//
// fragment_main
//
#version 460
precision highp float;
precision highp int;

layout(binding = 0, rg32ui) uniform highp uimage2DArray arg_0;
void textureStore_bd6602() {
  imageStore(arg_0, ivec3(ivec2(uvec2(1u)), 1), uvec4(1u));
}
void main() {
  textureStore_bd6602();
}
//
// compute_main
//
#version 460

layout(binding = 0, rg32ui) uniform highp uimage2DArray arg_0;
void textureStore_bd6602() {
  imageStore(arg_0, ivec3(ivec2(uvec2(1u)), 1), uvec4(1u));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  textureStore_bd6602();
}
