//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, rgba16f) uniform highp writeonly image2DArray arg_0;
void textureStore_a6a986() {
  uvec2 arg_1 = uvec2(1u);
  int arg_2 = 1;
  vec4 arg_3 = vec4(1.0f);
  int v = arg_2;
  vec4 v_1 = arg_3;
  imageStore(arg_0, ivec3(ivec2(arg_1), v), v_1);
}
void main() {
  textureStore_a6a986();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, rgba16f) uniform highp writeonly image2DArray arg_0;
void textureStore_a6a986() {
  uvec2 arg_1 = uvec2(1u);
  int arg_2 = 1;
  vec4 arg_3 = vec4(1.0f);
  int v = arg_2;
  vec4 v_1 = arg_3;
  imageStore(arg_0, ivec3(ivec2(arg_1), v), v_1);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  textureStore_a6a986();
}
