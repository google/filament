#version 450

layout(location = 0) flat in uint SNORM8;
layout(location = 1) flat in uint UNORM8;
layout(location = 2) flat in uint SNORM16;
layout(location = 3) flat in uint UNORM16;
layout(location = 4) flat in vec4 FP32;
layout(location = 0) out vec4 FP32Out;
layout(location = 1) out uint UNORM8Out;
layout(location = 2) out uint SNORM8Out;
layout(location = 3) out uint UNORM16Out;
layout(location = 4) out uint SNORM16Out;

void main()
{
	FP32Out = unpackUnorm4x8(UNORM8);
	FP32Out = unpackSnorm4x8(SNORM8);
	FP32Out.xy = unpackUnorm2x16(UNORM16);
	FP32Out.xy = unpackSnorm2x16(SNORM16);
	UNORM8Out = packUnorm4x8(FP32);
	SNORM8Out = packSnorm4x8(FP32);
	UNORM16Out = packUnorm2x16(FP32.xy);
	SNORM16Out = packSnorm2x16(FP32.zw);
}
