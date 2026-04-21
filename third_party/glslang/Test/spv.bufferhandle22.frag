#version 460
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require

layout(buffer_reference) buffer MyBuffer;

struct A {
    uint64_t a;
    uint b;
};
struct B {
    A a;
};

struct S1 { uint64_t x; };
struct S2 { float x; };
struct S3 { uvec3 x; };
struct S4 { uvec3 x[3][3][2][1][2]; };
struct S5 { mat3x3 x; };
struct S6 { MyBuffer x; };
struct S7 { B x; };

layout(set = 0, binding = 1) restrict readonly buffer SSBO {
	S1 s1;
	S2 s2;
	S3 s3;
	S4 s4;
	S5 s5;
	S6 s6;
	S7 s7;
} InBuffer;

layout(buffer_reference) buffer MyBuffer {
	S1 s1;
	S2 s2;
	S3 s3;
	S4 s4;
	S5 s5;
	S6 s6;
	S7 s7;
};

void main() {
	MyBuffer payload;
	payload.s1 = InBuffer.s1;
	payload.s2 = InBuffer.s2;
	payload.s3 = InBuffer.s3;
	payload.s4 = InBuffer.s4;
	payload.s5 = InBuffer.s5;
	payload.s6 = InBuffer.s6;
	payload.s7 = InBuffer.s7;
}