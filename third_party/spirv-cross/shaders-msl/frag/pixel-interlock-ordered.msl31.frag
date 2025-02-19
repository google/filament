#version 450
#extension GL_ARB_fragment_shader_interlock : require

layout(pixel_interlock_ordered) in;

layout(binding = 0, rgba8) uniform writeonly image2D img;
layout(binding = 1, r32ui) uniform uimage2D img2;
layout(binding = 2, rgba8) uniform readonly image2D img3;
layout(binding = 3) coherent buffer Buffer
{
	int foo;
	uint bar;
};
layout(binding = 4) buffer Buffer2
{
	uint quux;
};

layout(binding = 5, rgba8) uniform writeonly image2D img4;
layout(binding = 6) buffer Buffer3
{
	int baz;
};

void main()
{
	// Deliberately outside the critical section to test usage tracking.
	baz = 0;
	imageStore(img4, ivec2(1, 1), vec4(1.0, 0.0, 0.0, 1.0));
	beginInvocationInterlockARB();
	imageStore(img, ivec2(0, 0), imageLoad(img3, ivec2(0, 0)));
	imageAtomicAdd(img2, ivec2(0, 0), 1u);
	foo += 42;
	atomicAnd(bar, quux);
	endInvocationInterlockARB();
}
