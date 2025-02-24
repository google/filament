//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, rgba16ui) uniform highp writeonly uimage2DArray arg_0;
void textureStore_779d14() {
  uvec2 arg_1 = uvec2(1u);
  uint arg_2 = 1u;
  uvec4 arg_3 = uvec4(1u);
  uint v = arg_2;
  uvec4 v_1 = arg_3;
  ivec2 v_2 = ivec2(arg_1);
  imageStore(arg_0, ivec3(v_2, int(v)), v_1);
}
void main() {
  textureStore_779d14();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, rgba16ui) uniform highp writeonly uimage2DArray arg_0;
void textureStore_779d14() {
  uvec2 arg_1 = uvec2(1u);
  uint arg_2 = 1u;
  uvec4 arg_3 = uvec4(1u);
  uint v = arg_2;
  uvec4 v_1 = arg_3;
  ivec2 v_2 = ivec2(arg_1);
  imageStore(arg_0, ivec3(v_2, int(v)), v_1);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  textureStore_779d14();
}
