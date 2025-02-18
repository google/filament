//
// fragment_main
//
#version 460
precision highp float;
precision highp int;

layout(binding = 0, rg32f) uniform highp writeonly image3D arg_0;
void textureStore_fcbe66() {
  uvec3 arg_1 = uvec3(1u);
  vec4 arg_2 = vec4(1.0f);
  vec4 v = arg_2;
  imageStore(arg_0, ivec3(arg_1), v);
}
void main() {
  textureStore_fcbe66();
}
//
// compute_main
//
#version 460

layout(binding = 0, rg32f) uniform highp writeonly image3D arg_0;
void textureStore_fcbe66() {
  uvec3 arg_1 = uvec3(1u);
  vec4 arg_2 = vec4(1.0f);
  vec4 v = arg_2;
  imageStore(arg_0, ivec3(arg_1), v);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  textureStore_fcbe66();
}
