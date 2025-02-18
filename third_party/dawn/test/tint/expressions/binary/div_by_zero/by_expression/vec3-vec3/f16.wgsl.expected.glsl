#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f16vec3 a = f16vec3(1.0hf, 2.0hf, 3.0hf);
  f16vec3 b = f16vec3(0.0hf, 5.0hf, 0.0hf);
  f16vec3 r = (a / (b + b));
}
