//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, rgba8i) uniform highp writeonly iimage2D arg_0;
void textureStore_958353() {
  uint arg_1 = 1u;
  ivec4 arg_2 = ivec4(1);
  ivec4 v = arg_2;
  imageStore(arg_0, ivec2(uvec2(arg_1, 0u)), v);
}
void main() {
  textureStore_958353();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, rgba8i) uniform highp writeonly iimage2D arg_0;
void textureStore_958353() {
  uint arg_1 = 1u;
  ivec4 arg_2 = ivec4(1);
  ivec4 v = arg_2;
  imageStore(arg_0, ivec2(uvec2(arg_1, 0u)), v);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  textureStore_958353();
}
