#version 450
layout(depth_less) out float gl_FragDepth;

void main()
{
	gl_FragDepth = 0.5;
}
