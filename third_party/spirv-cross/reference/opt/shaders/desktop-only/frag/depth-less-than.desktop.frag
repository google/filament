#version 450
layout(depth_less) out float gl_FragDepth;
layout(early_fragment_tests) in;

void main()
{
    gl_FragDepth = 0.5;
}

