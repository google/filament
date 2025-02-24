//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, rgba16i) uniform highp writeonly iimage2DArray arg_0;
void textureStore_1f1ef8() {
  ivec2 arg_1 = ivec2(1);
  uint arg_2 = 1u;
  ivec4 arg_3 = ivec4(1);
  ivec2 v = arg_1;
  ivec4 v_1 = arg_3;
  imageStore(arg_0, ivec3(v, int(arg_2)), v_1);
}
void main() {
  textureStore_1f1ef8();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, rgba16i) uniform highp writeonly iimage2DArray arg_0;
void textureStore_1f1ef8() {
  ivec2 arg_1 = ivec2(1);
  uint arg_2 = 1u;
  ivec4 arg_3 = ivec4(1);
  ivec2 v = arg_1;
  ivec4 v_1 = arg_3;
  imageStore(arg_0, ivec3(v, int(arg_2)), v_1);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  textureStore_1f1ef8();
}
