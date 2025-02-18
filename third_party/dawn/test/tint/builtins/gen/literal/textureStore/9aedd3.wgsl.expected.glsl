//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, rgba8) uniform highp writeonly image3D arg_0;
void textureStore_9aedd3() {
  imageStore(arg_0, ivec3(uvec3(1u)), vec4(1.0f).zyxw);
}
void main() {
  textureStore_9aedd3();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, rgba8) uniform highp writeonly image3D arg_0;
void textureStore_9aedd3() {
  imageStore(arg_0, ivec3(uvec3(1u)), vec4(1.0f).zyxw);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  textureStore_9aedd3();
}
