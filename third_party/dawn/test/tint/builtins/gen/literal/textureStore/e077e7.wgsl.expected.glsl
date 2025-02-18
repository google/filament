//
// fragment_main
//
#version 460
precision highp float;
precision highp int;

layout(binding = 0, rg32i) uniform highp iimage2D arg_0;
void textureStore_e077e7() {
  imageStore(arg_0, ivec2(1, 0), ivec4(1));
}
void main() {
  textureStore_e077e7();
}
//
// compute_main
//
#version 460

layout(binding = 0, rg32i) uniform highp iimage2D arg_0;
void textureStore_e077e7() {
  imageStore(arg_0, ivec2(1, 0), ivec4(1));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  textureStore_e077e7();
}
