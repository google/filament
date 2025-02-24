//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, r32i) uniform highp writeonly iimage2DArray arg_0;
void textureStore_28a7ec() {
  uvec2 arg_1 = uvec2(1u);
  int arg_2 = 1;
  ivec4 arg_3 = ivec4(1);
  int v = arg_2;
  ivec4 v_1 = arg_3;
  imageStore(arg_0, ivec3(ivec2(arg_1), v), v_1);
}
void main() {
  textureStore_28a7ec();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, r32i) uniform highp writeonly iimage2DArray arg_0;
void textureStore_28a7ec() {
  uvec2 arg_1 = uvec2(1u);
  int arg_2 = 1;
  ivec4 arg_3 = ivec4(1);
  int v = arg_2;
  ivec4 v_1 = arg_3;
  imageStore(arg_0, ivec3(ivec2(arg_1), v), v_1);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  textureStore_28a7ec();
}
