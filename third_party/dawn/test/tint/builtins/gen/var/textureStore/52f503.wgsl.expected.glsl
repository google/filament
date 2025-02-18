//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, rgba32i) uniform highp writeonly iimage2D arg_0;
void textureStore_52f503() {
  uvec2 arg_1 = uvec2(1u);
  ivec4 arg_2 = ivec4(1);
  ivec4 v = arg_2;
  imageStore(arg_0, ivec2(arg_1), v);
}
void main() {
  textureStore_52f503();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, rgba32i) uniform highp writeonly iimage2D arg_0;
void textureStore_52f503() {
  uvec2 arg_1 = uvec2(1u);
  ivec4 arg_2 = ivec4(1);
  ivec4 v = arg_2;
  imageStore(arg_0, ivec2(arg_1), v);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  textureStore_52f503();
}
