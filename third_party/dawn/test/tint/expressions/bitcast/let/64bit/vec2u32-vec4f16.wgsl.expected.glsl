#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

f16vec4 tint_bitcast_to_f16(uvec2 src) {
  return f16vec4(unpackFloat2x16(src.x), unpackFloat2x16(src.y));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uvec2 a = uvec2(1073757184u, 3288351232u);
  f16vec4 b = tint_bitcast_to_f16(a);
}
