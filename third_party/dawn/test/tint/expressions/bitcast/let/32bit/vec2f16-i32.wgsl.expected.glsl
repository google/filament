#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

int tint_bitcast_from_f16(f16vec2 src) {
  return int(packFloat2x16(src));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f16vec2 a = f16vec2(1.0hf, 2.0hf);
  int b = tint_bitcast_from_f16(a);
}
