#version 450
layout(depth_greater) out float gl_FragDepth;

void main()
{
	gl_FragDepth = 0.5;
}
