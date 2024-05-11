#version 430
layout(depth_less) out float gl_FragDepth;

layout(location = 0) out vec4 color_out;

void main()
{
    color_out = vec4(1.0, 0.0, 0.0, 1.0);
    gl_FragDepth = 0.699999988079071044921875;
}
