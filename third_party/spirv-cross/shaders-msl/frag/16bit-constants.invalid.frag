#version 450 core

#extension GL_AMD_gpu_shader_int16 : require
#extension GL_AMD_gpu_shader_half_float : require

layout(location = 0) out float16_t foo;
layout(location = 1) out int16_t bar;
layout(location = 2) out uint16_t baz;

void main() {
	foo = 1.0hf;
	bar = 2s;
	baz = 3us;
}
