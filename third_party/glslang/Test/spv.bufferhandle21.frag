#version 460
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require

struct Data {
	uint a;
	uint b;
	uint c;
	int d;
	uint e;
};

layout(set = 0, binding = 1) restrict readonly buffer SSBO_1 {
	Data data;
} InBuffer;

layout(buffer_reference) buffer MyBuffer {
	Data data;
};

void main() {
	MyBuffer x;
	x.data = InBuffer.data;
}