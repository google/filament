#version 450
#extension GL_ARB_fragment_shader_interlock : require

layout(pixel_interlock_ordered) in;

layout(binding = 0, rgba8) uniform writeonly image2D img;
layout(binding = 1, r32ui) uniform uimage2D img2;
layout(binding = 2) coherent buffer Buffer
{
	int foo;
	uint bar;
};

void main()
{
	beginInvocationInterlockARB();
	imageStore(img, ivec2(0, 0), vec4(1.0, 0.0, 0.0, 1.0));
	imageAtomicAdd(img2, ivec2(0, 0), 1u);
	foo += 42;
	atomicAnd(bar, 0xff);
	endInvocationInterlockARB();
}
