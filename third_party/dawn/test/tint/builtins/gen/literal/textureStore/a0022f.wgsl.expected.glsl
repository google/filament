//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, r32i) uniform highp iimage2DArray arg_0;
void textureStore_a0022f() {
  imageStore(arg_0, ivec3(ivec2(1), 1), ivec4(1));
}
void main() {
  textureStore_a0022f();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, r32i) uniform highp iimage2DArray arg_0;
void textureStore_a0022f() {
  imageStore(arg_0, ivec3(ivec2(1), 1), ivec4(1));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  textureStore_a0022f();
}
