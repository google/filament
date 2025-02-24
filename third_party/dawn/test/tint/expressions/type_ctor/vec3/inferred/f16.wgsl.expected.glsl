#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

f16vec3 v = f16vec3(0.0hf, 1.0hf, 2.0hf);
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
