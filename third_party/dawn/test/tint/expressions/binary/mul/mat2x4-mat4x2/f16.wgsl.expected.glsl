#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f16mat2x4 a = f16mat2x4(f16vec4(1.0hf, 2.0hf, 3.0hf, 4.0hf), f16vec4(5.0hf, 6.0hf, 7.0hf, 8.0hf));
  f16mat4x2 b = f16mat4x2(f16vec2(-1.0hf, -2.0hf), f16vec2(-3.0hf, -4.0hf), f16vec2(-5.0hf, -6.0hf), f16vec2(-7.0hf, -8.0hf));
  f16mat4 r = (a * b);
}
