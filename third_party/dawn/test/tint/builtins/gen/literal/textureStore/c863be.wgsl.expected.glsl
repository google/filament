//
// fragment_main
//
#version 460
precision highp float;
precision highp int;

layout(binding = 0, rg32f) uniform highp writeonly image2DArray arg_0;
void textureStore_c863be() {
  imageStore(arg_0, ivec3(ivec2(1), 1), vec4(1.0f));
}
void main() {
  textureStore_c863be();
}
//
// compute_main
//
#version 460

layout(binding = 0, rg32f) uniform highp writeonly image2DArray arg_0;
void textureStore_c863be() {
  imageStore(arg_0, ivec3(ivec2(1), 1), vec4(1.0f));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  textureStore_c863be();
}
