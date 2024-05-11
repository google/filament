#version 450

layout(location = 0) flat in uint FP16;
layout(location = 1) flat in vec2 FP32;
layout(location = 0) out vec2 FP32Out;
layout(location = 1) out uint FP16Out;

void main()
{
	FP32Out = unpackHalf2x16(FP16);
	FP16Out = packHalf2x16(FP32);
}
