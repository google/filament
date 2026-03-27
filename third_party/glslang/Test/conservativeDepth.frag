#version 300 es
#extension GL_EXT_conservative_depth: require
layout (depth_any) out highp float gl_FragDepth;
void main() {
    gl_FragDepth = 1.0;
}
