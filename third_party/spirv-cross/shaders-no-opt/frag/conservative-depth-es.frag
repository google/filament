#version 310 es
#extension GL_EXT_conservative_depth : enable

layout(depth_greater) out highp float gl_FragDepth;

void main() {
    gl_FragDepth = 1.0;
}
