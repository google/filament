//
// fragment_main
//
#version 460
precision highp float;
precision highp int;

layout(binding = 0, rg32ui) uniform highp writeonly uimage2D arg_0;
void textureStore_506a71() {
  uvec2 arg_1 = uvec2(1u);
  uvec4 arg_2 = uvec4(1u);
  uvec4 v = arg_2;
  imageStore(arg_0, ivec2(arg_1), v);
}
void main() {
  textureStore_506a71();
}
//
// compute_main
//
#version 460

layout(binding = 0, rg32ui) uniform highp writeonly uimage2D arg_0;
void textureStore_506a71() {
  uvec2 arg_1 = uvec2(1u);
  uvec4 arg_2 = uvec4(1u);
  uvec4 v = arg_2;
  imageStore(arg_0, ivec2(arg_1), v);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  textureStore_506a71();
}
