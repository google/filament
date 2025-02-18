#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

f16mat2x4 u = f16mat2x4(f16vec4(1.0hf, 2.0hf, 3.0hf, 4.0hf), f16vec4(5.0hf, 6.0hf, 7.0hf, 8.0hf));
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
