//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, rgba32i) uniform highp writeonly iimage2D arg_0;
void textureStore_752da6() {
  imageStore(arg_0, ivec2(1), ivec4(1));
}
void main() {
  textureStore_752da6();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, rgba32i) uniform highp writeonly iimage2D arg_0;
void textureStore_752da6() {
  imageStore(arg_0, ivec2(1), ivec4(1));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  textureStore_752da6();
}
