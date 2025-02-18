//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, r32i) uniform highp iimage3D arg_0;
void textureStore_24e6b7() {
  imageStore(arg_0, ivec3(1), ivec4(1));
}
void main() {
  textureStore_24e6b7();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, r32i) uniform highp iimage3D arg_0;
void textureStore_24e6b7() {
  imageStore(arg_0, ivec3(1), ivec4(1));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  textureStore_24e6b7();
}
