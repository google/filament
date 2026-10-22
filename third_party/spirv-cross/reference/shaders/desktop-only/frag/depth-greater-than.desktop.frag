#version 450
#extension GL_ARB_conservative_depth : require
layout(depth_greater) out float gl_FragDepth;
layout(early_fragment_tests) in;

void main()
{
    gl_FragDepth = 0.5;
}

