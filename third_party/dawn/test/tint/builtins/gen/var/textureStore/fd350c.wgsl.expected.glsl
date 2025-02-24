//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, rgba16ui) uniform highp writeonly uimage2DArray arg_0;
void textureStore_fd350c() {
  ivec2 arg_1 = ivec2(1);
  uint arg_2 = 1u;
  uvec4 arg_3 = uvec4(1u);
  ivec2 v = arg_1;
  uvec4 v_1 = arg_3;
  imageStore(arg_0, ivec3(v, int(arg_2)), v_1);
}
void main() {
  textureStore_fd350c();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, rgba16ui) uniform highp writeonly uimage2DArray arg_0;
void textureStore_fd350c() {
  ivec2 arg_1 = ivec2(1);
  uint arg_2 = 1u;
  uvec4 arg_3 = uvec4(1u);
  ivec2 v = arg_1;
  uvec4 v_1 = arg_3;
  imageStore(arg_0, ivec3(v, int(arg_2)), v_1);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  textureStore_fd350c();
}
