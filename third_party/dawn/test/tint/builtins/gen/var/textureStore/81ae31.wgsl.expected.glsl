//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, r32f) uniform highp image2DArray arg_0;
void textureStore_81ae31() {
  ivec2 arg_1 = ivec2(1);
  int arg_2 = 1;
  vec4 arg_3 = vec4(1.0f);
  vec4 v = arg_3;
  imageStore(arg_0, ivec3(arg_1, arg_2), v);
}
void main() {
  textureStore_81ae31();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, r32f) uniform highp image2DArray arg_0;
void textureStore_81ae31() {
  ivec2 arg_1 = ivec2(1);
  int arg_2 = 1;
  vec4 arg_3 = vec4(1.0f);
  vec4 v = arg_3;
  imageStore(arg_0, ivec3(arg_1, arg_2), v);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  textureStore_81ae31();
}
