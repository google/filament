#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

f16vec4 tint_bitcast_to_f16(ivec2 src) {
  uvec2 v = uvec2(src);
  return f16vec4(unpackFloat2x16(v.x), unpackFloat2x16(v.y));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  ivec2 a = ivec2(1073757184, -1006616064);
  f16vec4 b = tint_bitcast_to_f16(a);
}
