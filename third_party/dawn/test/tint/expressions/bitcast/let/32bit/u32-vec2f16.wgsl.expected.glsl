#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

f16vec2 tint_bitcast_to_f16(uint src) {
  return unpackFloat2x16(src);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint a = 1073757184u;
  f16vec2 b = tint_bitcast_to_f16(a);
}
