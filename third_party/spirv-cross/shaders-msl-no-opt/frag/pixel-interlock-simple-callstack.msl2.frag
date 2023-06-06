#version 450
#extension GL_ARB_fragment_shader_interlock : require
layout(pixel_interlock_ordered) in;

layout(set = 0, binding = 0, std430) buffer SSBO0
{
	uint values0[];
};

layout(set = 0, binding = 1, std430) buffer SSBO1
{
	uint values1[];
};

void callee2()
{
	values1[int(gl_FragCoord.x)] += 1;
}

void callee()
{
	values0[int(gl_FragCoord.x)] += 1;
	callee2();
}

void main()
{
	beginInvocationInterlockARB();
	callee();
	endInvocationInterlockARB();
}
