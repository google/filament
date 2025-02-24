//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, r32f) uniform highp image2D arg_0;
void textureStore_6e6cc0() {
  imageStore(arg_0, ivec2(1, 0), vec4(1.0f));
}
void main() {
  textureStore_6e6cc0();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, r32f) uniform highp image2D arg_0;
void textureStore_6e6cc0() {
  imageStore(arg_0, ivec2(1, 0), vec4(1.0f));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  textureStore_6e6cc0();
}
