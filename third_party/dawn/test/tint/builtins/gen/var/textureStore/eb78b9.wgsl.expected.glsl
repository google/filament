//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, r32i) uniform highp writeonly iimage3D arg_0;
void textureStore_eb78b9() {
  ivec3 arg_1 = ivec3(1);
  ivec4 arg_2 = ivec4(1);
  imageStore(arg_0, arg_1, arg_2);
}
void main() {
  textureStore_eb78b9();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, r32i) uniform highp writeonly iimage3D arg_0;
void textureStore_eb78b9() {
  ivec3 arg_1 = ivec3(1);
  ivec4 arg_2 = ivec4(1);
  imageStore(arg_0, arg_1, arg_2);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  textureStore_eb78b9();
}
