#version 450
#extension GL_ARB_conservative_depth : enable

layout(depth_greater) out float gl_FragDepth;

void main() {
    gl_FragDepth = 1;
}
