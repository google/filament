//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, r32ui) uniform highp writeonly uimage2D arg_0;
void textureStore_6cff2e() {
  ivec2 arg_1 = ivec2(1);
  uvec4 arg_2 = uvec4(1u);
  imageStore(arg_0, arg_1, arg_2);
}
void main() {
  textureStore_6cff2e();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, r32ui) uniform highp writeonly uimage2D arg_0;
void textureStore_6cff2e() {
  ivec2 arg_1 = ivec2(1);
  uvec4 arg_2 = uvec4(1u);
  imageStore(arg_0, arg_1, arg_2);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  textureStore_6cff2e();
}
