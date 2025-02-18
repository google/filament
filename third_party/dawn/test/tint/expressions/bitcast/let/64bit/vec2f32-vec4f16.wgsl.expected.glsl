#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

f16vec4 tint_bitcast_to_f16(vec2 src) {
  return f16vec4(unpackFloat2x16(floatBitsToUint(src).x), unpackFloat2x16(floatBitsToUint(src).y));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  vec2 a = vec2(2.003662109375f, -513.03125f);
  f16vec4 b = tint_bitcast_to_f16(a);
}
