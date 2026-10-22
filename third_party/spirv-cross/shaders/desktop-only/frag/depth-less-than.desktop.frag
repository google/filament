#version 450
layout(early_fragment_tests) in;
layout(depth_less) out float gl_FragDepth;

void main()
{
	gl_FragDepth = 0.5;
}
