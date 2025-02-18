//
// fragment_main
//
#version 460
precision highp float;
precision highp int;

layout(binding = 0, rg32f) uniform highp image2D arg_0;
void textureStore_5b17eb() {
  imageStore(arg_0, ivec2(uvec2(1u)), vec4(1.0f));
}
void main() {
  textureStore_5b17eb();
}
//
// compute_main
//
#version 460

layout(binding = 0, rg32f) uniform highp image2D arg_0;
void textureStore_5b17eb() {
  imageStore(arg_0, ivec2(uvec2(1u)), vec4(1.0f));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  textureStore_5b17eb();
}
