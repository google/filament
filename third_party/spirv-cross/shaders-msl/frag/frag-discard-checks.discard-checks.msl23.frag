#version 450

layout(set=0, binding=0, std430) buffer foo_t
{
	float x;
	uint y;
} foo;

layout(r32ui, set=0, binding=1) uniform uimage2D bar;

layout(location=0) out vec4 fragColor;

vec4 frag_body() {
	foo.x = 1.0f;
	atomicExchange(foo.y, 0);
	if (int(gl_FragCoord.x) == 3)
		discard;
	imageStore(bar, ivec2(gl_FragCoord.xy), uvec4(1));
	atomicAdd(foo.y, 42);
	imageAtomicOr(bar, ivec2(gl_FragCoord.xy), 0x3e);
	atomicAnd(foo.y, 0xffff);
	atomicXor(foo.y, 0xffffff00);
	atomicMin(foo.y, 1);
	imageAtomicMax(bar, ivec2(gl_FragCoord.xy), 100);
	imageAtomicCompSwap(bar, ivec2(gl_FragCoord.xy), 100, 42);
	return vec4(1.0f, 0.0f, 0.0f, 1.0f);
}

void main() {
	fragColor = frag_body();
}

