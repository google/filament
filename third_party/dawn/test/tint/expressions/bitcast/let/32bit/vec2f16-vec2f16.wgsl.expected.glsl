#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f16vec2 a = f16vec2(1.0hf, 2.0hf);
  f16vec2 b = a;
}
